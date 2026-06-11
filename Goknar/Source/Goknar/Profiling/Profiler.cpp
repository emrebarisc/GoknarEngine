#include "pch.h"

#ifdef GOKNAR_DEBUG

#include "Goknar/Profiling/Profiler.h"
#include "Goknar/Profiling/ProfileScope.h"

#include <array>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>

namespace Goknar
{
namespace Debug
{
	namespace
	{
		constexpr std::uint32_t MaxEventsPerThread = 16384;
		constexpr std::uint32_t ProfilePageCount = 2;

		struct ProfileEventPage
		{
			std::array<ProfileEvent, MaxEventsPerThread> events{};
			std::uint32_t count{ 0 };
			std::uint64_t droppedEventCount{ 0 };

			void Clear()
			{
				count = 0;
				droppedEventCount = 0;
			}
		};
	}

	struct ProfileThreadBuffer
	{
		std::array<ProfileEventPage, ProfilePageCount> pages{};
		std::atomic<std::uint32_t> activePageIndex{ 0 };
		std::atomic<std::uint32_t> activeWriterCount{ 0 };
		std::atomic<std::uint64_t> totalDroppedEventCount{ 0 };
		std::uint32_t depth{ 0 };
		std::uint32_t threadIndex{ 0 };
		std::uint64_t threadId{ 0 };
		char name[64]{};
	};

	namespace
	{
		struct ProfilerState
		{
			std::atomic<bool> isEnabled{ false };
			std::atomic<std::uint64_t> captureRequestedFrameCount{ 0 };
			std::atomic<bool> captureFramesActive{ false };
			std::atomic<std::uint64_t> captureRemainingFrameCount{ 0 };
			std::atomic<std::uint64_t> frameIndex{ 0 };
			std::mutex threadBuffersMutex{};
			std::vector<std::unique_ptr<ProfileThreadBuffer>> threadBuffers{};
			std::uint32_t nextThreadIndex{ 0 };
			std::chrono::steady_clock::time_point origin{ std::chrono::steady_clock::now() };
		};

		ProfilerState& GetState()
		{
			static ProfilerState state;
			return state;
		}

		thread_local ProfileThreadBuffer* CurrentThreadBuffer = nullptr;

		std::uint64_t GetCurrentThreadId()
		{
			return static_cast<std::uint64_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
		}

		void CopyThreadName(char* destination, std::size_t destinationSize, const char* source)
		{
			if (!destination || destinationSize == 0)
			{
				return;
			}

			if (!source || source[0] == '\0')
			{
				source = "Unnamed Thread";
			}

			std::strncpy(destination, source, destinationSize - 1);
			destination[destinationSize - 1] = '\0';
		}

		void WriteJsonString(std::ostream& stream, const char* value)
		{
			stream << '"';

			if (value)
			{
				for (const unsigned char character : std::string(value))
				{
					switch (character)
					{
					case '\\':
						stream << "\\\\";
						break;
					case '"':
						stream << "\\\"";
						break;
					case '\b':
						stream << "\\b";
						break;
					case '\f':
						stream << "\\f";
						break;
					case '\n':
						stream << "\\n";
						break;
					case '\r':
						stream << "\\r";
						break;
					case '\t':
						stream << "\\t";
						break;
					default:
						if (character < 0x20)
						{
							const std::ios::fmtflags flags = stream.flags();
							const char fill = stream.fill();
							stream << "\\u"
								<< std::hex << std::setw(4) << std::setfill('0')
								<< static_cast<int>(character);
							stream.flags(flags);
							stream.fill(fill);
						}
						else
						{
							stream << static_cast<char>(character);
						}
						break;
					}
				}
			}

			stream << '"';
		}

		void WriteJsonString(std::ostream& stream, const std::string& value)
		{
			WriteJsonString(stream, value.c_str());
		}

		double ToMicroseconds(std::uint64_t nanoseconds)
		{
			return static_cast<double>(nanoseconds) / 1000.0;
		}

		void ClearRecordedEvents()
		{
			ProfilerState& state = GetState();

			std::vector<ProfileThreadBuffer*> buffers;
			{
				std::lock_guard<std::mutex> lock(state.threadBuffersMutex);
				buffers.reserve(state.threadBuffers.size());
				for (const std::unique_ptr<ProfileThreadBuffer>& threadBuffer : state.threadBuffers)
				{
					buffers.push_back(threadBuffer.get());
				}
			}

			for (ProfileThreadBuffer* threadBuffer : buffers)
			{
				while (threadBuffer->activeWriterCount.load(std::memory_order_acquire) != 0)
				{
					std::this_thread::yield();
				}

				for (ProfileEventPage& page : threadBuffer->pages)
				{
					page.Clear();
				}

				threadBuffer->totalDroppedEventCount.store(0, std::memory_order_release);
			}
		}
	}

	void Profiler::SetEnabled(bool isEnabled)
	{
		ProfilerState& state = GetState();
		state.captureRequestedFrameCount.store(0, std::memory_order_release);
		state.captureFramesActive.store(false, std::memory_order_release);
		state.captureRemainingFrameCount.store(0, std::memory_order_release);
		state.isEnabled.store(isEnabled, std::memory_order_release);
	}

	bool Profiler::IsEnabled()
	{
		return GetState().isEnabled.load(std::memory_order_acquire);
	}

	bool Profiler::IsEnabledRelaxed()
	{
		return GetState().isEnabled.load(std::memory_order_relaxed);
	}

	void Profiler::SetCurrentThreadName(const char* name)
	{
		ProfileThreadBuffer* threadBuffer = GetThreadBuffer();
		CopyThreadName(threadBuffer->name, sizeof(threadBuffer->name), name);
	}

	void Profiler::Clear()
	{
		ProfilerState& state = GetState();
		const bool wasEnabled = state.isEnabled.exchange(false, std::memory_order_acq_rel);

		state.captureRequestedFrameCount.store(0, std::memory_order_release);
		state.captureFramesActive.store(false, std::memory_order_release);
		state.captureRemainingFrameCount.store(0, std::memory_order_release);
		ClearRecordedEvents();
		state.frameIndex.store(0, std::memory_order_release);
		state.isEnabled.store(wasEnabled, std::memory_order_release);
	}

	void Profiler::CaptureFrames(std::uint64_t frameCount)
	{
		if (frameCount == 0)
		{
			return;
		}

		ProfilerState& state = GetState();
		state.isEnabled.store(false, std::memory_order_release);
		state.captureFramesActive.store(false, std::memory_order_release);
		state.captureRemainingFrameCount.store(0, std::memory_order_release);
		state.captureRequestedFrameCount.store(frameCount, std::memory_order_release);
	}

	void Profiler::CaptureOneFrame()
	{
		CaptureFrames(1);
	}

	bool Profiler::IsCaptureFramesPending()
	{
		return GetState().captureRequestedFrameCount.load(std::memory_order_acquire) > 0;
	}

	bool Profiler::IsCapturingFrames()
	{
		return GetState().captureFramesActive.load(std::memory_order_acquire);
	}

	std::uint64_t Profiler::GetRemainingCaptureFrameCount()
	{
		const ProfilerState& state = GetState();
		const std::uint64_t pendingFrameCount = state.captureRequestedFrameCount.load(std::memory_order_acquire);
		if (pendingFrameCount > 0)
		{
			return pendingFrameCount;
		}

		return state.captureRemainingFrameCount.load(std::memory_order_acquire);
	}

	bool Profiler::IsCaptureOneFramePending()
	{
		return IsCaptureFramesPending();
	}

	bool Profiler::IsCapturingOneFrame()
	{
		return IsCapturingFrames();
	}

	ProfileSnapshot Profiler::CaptureSnapshot()
	{
		ProfilerState& state = GetState();
		ProfileSnapshot snapshot;
		snapshot.frameIndex = state.frameIndex.load(std::memory_order_acquire);

		std::vector<ProfileThreadBuffer*> buffers;
		{
			std::lock_guard<std::mutex> lock(state.threadBuffersMutex);
			buffers.reserve(state.threadBuffers.size());
			for (const std::unique_ptr<ProfileThreadBuffer>& threadBuffer : state.threadBuffers)
			{
				buffers.push_back(threadBuffer.get());
			}
		}

		for (ProfileThreadBuffer* threadBuffer : buffers)
		{
			ProfileThreadInfo threadInfo;
			threadInfo.index = threadBuffer->threadIndex;
			threadInfo.id = threadBuffer->threadId;
			threadInfo.droppedEventCount = threadBuffer->totalDroppedEventCount.load(std::memory_order_acquire);
			threadInfo.name = threadBuffer->name;
			snapshot.threads.push_back(threadInfo);

			const std::uint32_t capturePageIndex = threadBuffer->activePageIndex.exchange(
				(threadBuffer->activePageIndex.load(std::memory_order_acquire) + 1) % ProfilePageCount,
				std::memory_order_acq_rel);

			while (threadBuffer->activeWriterCount.load(std::memory_order_acquire) != 0)
			{
				std::this_thread::yield();
			}

			ProfileEventPage& page = threadBuffer->pages[capturePageIndex];
			snapshot.events.reserve(snapshot.events.size() + page.count);
			for (std::uint32_t eventIndex = 0; eventIndex < page.count; ++eventIndex)
			{
				const ProfileEvent& event = page.events[eventIndex];
				snapshot.events.push_back(event);

				if (snapshot.firstTimestampNs == 0 || event.startTimeNs < snapshot.firstTimestampNs)
				{
					snapshot.firstTimestampNs = event.startTimeNs;
				}

				if (snapshot.lastTimestampNs < event.endTimeNs)
				{
					snapshot.lastTimestampNs = event.endTimeNs;
				}
			}

			snapshot.droppedEventCount += page.droppedEventCount;
			page.Clear();
		}

		std::sort(
			snapshot.events.begin(),
			snapshot.events.end(),
			[](const ProfileEvent& left, const ProfileEvent& right)
			{
				if (left.startTimeNs != right.startTimeNs)
				{
					return left.startTimeNs < right.startTimeNs;
				}

				return left.durationNs > right.durationNs;
			});

		return snapshot;
	}

	bool Profiler::ExportChromeTrace(const std::string& path)
	{
		return WriteChromeTrace(path, CaptureSnapshot());
	}

	bool Profiler::WriteChromeTrace(const std::string& path, const ProfileSnapshot& snapshot)
	{
		std::ofstream output(path, std::ios::out | std::ios::trunc);
		if (!output.is_open())
		{
			return false;
		}

		constexpr std::uint64_t ProcessId = 1;
		bool needsComma = false;

		auto beginTraceEvent = [&]()
		{
			if (needsComma)
			{
				output << ",\n";
			}
			else
			{
				needsComma = true;
			}
			output << "    {";
		};

		output << "{\n";
		output << "  \"traceEvents\": [\n";

		beginTraceEvent();
		output << "\"name\":\"process_name\",\"ph\":\"M\",\"pid\":" << ProcessId
			<< ",\"tid\":0,\"args\":{\"name\":\"Goknar\"}}";

		for (const ProfileThreadInfo& threadInfo : snapshot.threads)
		{
			beginTraceEvent();
			output << "\"name\":\"thread_name\",\"ph\":\"M\",\"pid\":" << ProcessId
				<< ",\"tid\":" << threadInfo.id << ",\"args\":{\"name\":";
			WriteJsonString(output, threadInfo.name);
			output << "}}";
		}

		output << std::fixed << std::setprecision(3);
		for (const ProfileEvent& event : snapshot.events)
		{
			beginTraceEvent();
			output << "\"cat\":\"cpu\",\"name\":";
			WriteJsonString(output, event.name ? event.name : "Unnamed Scope");
			output << ",\"ph\":\"X\",\"ts\":" << ToMicroseconds(event.startTimeNs)
				<< ",\"dur\":" << ToMicroseconds(event.durationNs)
				<< ",\"pid\":" << ProcessId
				<< ",\"tid\":" << event.threadId
				<< ",\"args\":{\"frame\":" << event.frameIndex
				<< ",\"depth\":" << event.depth
				<< ",\"threadIndex\":" << event.threadIndex;
			if (event.filePath && event.filePath[0] != '\0')
			{
				output << ",\"file\":";
				WriteJsonString(output, event.filePath);
				output << ",\"line\":" << event.lineNumber;
			}
			output << "}}";
		}

		output << "\n  ],\n";
		output << "  \"displayTimeUnit\": \"ms\",\n";
		output << "  \"otherData\": {\n";
		output << "    \"source\": \"Goknar CPU Profiler\",\n";
		output << "    \"eventCount\": " << snapshot.events.size() << ",\n";
		output << "    \"droppedEventCount\": " << snapshot.droppedEventCount << ",\n";
		output << "    \"frameIndex\": " << snapshot.frameIndex << "\n";
		output << "  }\n";
		output << "}\n";

		return true;
	}

	std::uint64_t Profiler::BeginFrame(const char* name)
	{
		(void)name;
		ProfilerState& state = GetState();
		const std::uint64_t requestedFrameCount = state.captureRequestedFrameCount.exchange(0, std::memory_order_acq_rel);
		if (requestedFrameCount > 0)
		{
			ClearRecordedEvents();
			state.frameIndex.store(0, std::memory_order_release);
		}

		const std::uint64_t frameIndex = state.frameIndex.fetch_add(1, std::memory_order_acq_rel) + 1;
		if (requestedFrameCount > 0)
		{
			state.captureRemainingFrameCount.store(requestedFrameCount, std::memory_order_release);
			state.captureFramesActive.store(true, std::memory_order_release);
			state.isEnabled.store(true, std::memory_order_release);
		}

		return frameIndex;
	}

	void Profiler::EndFrame(std::uint64_t frameIndex)
	{
		(void)frameIndex;
		ProfilerState& state = GetState();
		if (!state.captureFramesActive.load(std::memory_order_acquire))
		{
			return;
		}

		std::uint64_t remainingFrameCount = state.captureRemainingFrameCount.load(std::memory_order_acquire);
		while (remainingFrameCount > 0)
		{
			if (state.captureRemainingFrameCount.compare_exchange_weak(
				remainingFrameCount,
				remainingFrameCount - 1,
				std::memory_order_acq_rel,
				std::memory_order_acquire))
			{
				break;
			}
		}

		if (remainingFrameCount <= 1)
		{
			state.isEnabled.store(false, std::memory_order_release);
			state.captureFramesActive.store(false, std::memory_order_release);
			state.captureRemainingFrameCount.store(0, std::memory_order_release);
		}
	}

	std::uint64_t Profiler::GetCurrentFrameIndex()
	{
		return GetState().frameIndex.load(std::memory_order_acquire);
	}

	std::uint32_t Profiler::GetMaxEventsPerThread()
	{
		return MaxEventsPerThread;
	}

	bool Profiler::ShouldStartFrame()
	{
		const ProfilerState& state = GetState();
		return state.isEnabled.load(std::memory_order_relaxed) ||
			state.captureRequestedFrameCount.load(std::memory_order_relaxed) > 0 ||
			state.captureFramesActive.load(std::memory_order_relaxed);
	}

	std::uint64_t Profiler::GetTimestampNs()
	{
		const ProfilerState& state = GetState();
		const auto now = std::chrono::steady_clock::now();
		return static_cast<std::uint64_t>(
			std::chrono::duration_cast<std::chrono::nanoseconds>(now - state.origin).count());
	}

	ProfileThreadBuffer* Profiler::GetThreadBuffer()
	{
		if (CurrentThreadBuffer)
		{
			return CurrentThreadBuffer;
		}

		ProfilerState& state = GetState();
		std::unique_ptr<ProfileThreadBuffer> newThreadBuffer = std::make_unique<ProfileThreadBuffer>();
		newThreadBuffer->threadId = GetCurrentThreadId();

		ProfileThreadBuffer* threadBuffer = newThreadBuffer.get();
		{
			std::lock_guard<std::mutex> lock(state.threadBuffersMutex);
			threadBuffer->threadIndex = state.nextThreadIndex++;
			std::snprintf(threadBuffer->name, sizeof(threadBuffer->name), "Thread %u", threadBuffer->threadIndex);
			state.threadBuffers.push_back(std::move(newThreadBuffer));
		}

		CurrentThreadBuffer = threadBuffer;
		return CurrentThreadBuffer;
	}

	void Profiler::RecordEvent(
		ProfileThreadBuffer* threadBuffer,
		const char* name,
		const char* filePath,
		std::uint32_t lineNumber,
		std::uint64_t startTimeNs,
		std::uint64_t endTimeNs,
		std::uint32_t depth,
		std::uint64_t frameIndex)
	{
		if (!threadBuffer || !name || endTimeNs < startTimeNs)
		{
			return;
		}

		threadBuffer->activeWriterCount.fetch_add(1, std::memory_order_acquire);

		const std::uint32_t pageIndex = threadBuffer->activePageIndex.load(std::memory_order_acquire);
		ProfileEventPage& page = threadBuffer->pages[pageIndex];
		if (page.count < MaxEventsPerThread)
		{
			ProfileEvent& event = page.events[page.count++];
			event.name = name;
			event.filePath = filePath;
			event.startTimeNs = startTimeNs;
			event.endTimeNs = endTimeNs;
			event.durationNs = endTimeNs - startTimeNs;
			event.threadId = threadBuffer->threadId;
			event.threadIndex = threadBuffer->threadIndex;
			event.frameIndex = frameIndex;
			event.depth = depth;
			event.lineNumber = lineNumber;
		}
		else
		{
			++page.droppedEventCount;
			threadBuffer->totalDroppedEventCount.fetch_add(1, std::memory_order_relaxed);
		}

		threadBuffer->activeWriterCount.fetch_sub(1, std::memory_order_release);
	}

	ProfileScope::ProfileScope(const char* name, const char* filePath, std::uint32_t lineNumber) noexcept :
		name_(name),
		filePath_(filePath),
		lineNumber_(lineNumber)
	{
		if (!Profiler::IsEnabledRelaxed())
		{
			return;
		}

		threadBuffer_ = Profiler::GetThreadBuffer();
		frameIndex_ = Profiler::GetCurrentFrameIndex();
		depth_ = threadBuffer_->depth++;
		startTimeNs_ = Profiler::GetTimestampNs();
		isActive_ = true;
	}

	ProfileScope::~ProfileScope() noexcept
	{
		if (!isActive_)
		{
			return;
		}

		const std::uint64_t endTimeNs = Profiler::GetTimestampNs();
		if (0 < threadBuffer_->depth)
		{
			--threadBuffer_->depth;
		}

		Profiler::RecordEvent(threadBuffer_, name_, filePath_, lineNumber_, startTimeNs_, endTimeNs, depth_, frameIndex_);
	}

	ProfileFrameScope::ProfileFrameScope(const char* name, const char* filePath, std::uint32_t lineNumber) noexcept :
		name_(name),
		filePath_(filePath),
		lineNumber_(lineNumber)
	{
		if (!Profiler::ShouldStartFrame())
		{
			return;
		}

		frameIndex_ = Profiler::BeginFrame(name);
		threadBuffer_ = Profiler::GetThreadBuffer();
		depth_ = threadBuffer_->depth++;
		startTimeNs_ = Profiler::GetTimestampNs();
		isActive_ = true;
	}

	ProfileFrameScope::~ProfileFrameScope() noexcept
	{
		if (!isActive_)
		{
			return;
		}

		const std::uint64_t endTimeNs = Profiler::GetTimestampNs();
		if (0 < threadBuffer_->depth)
		{
			--threadBuffer_->depth;
		}

		Profiler::RecordEvent(threadBuffer_, name_, filePath_, lineNumber_, startTimeNs_, endTimeNs, depth_, frameIndex_);
		Profiler::EndFrame(frameIndex_);
	}
}
}

#endif
