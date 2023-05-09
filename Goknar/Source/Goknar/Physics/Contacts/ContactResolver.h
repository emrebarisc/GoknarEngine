#ifndef __CONTACTRESOLVER_H__
#define __CONTACTRESOLVER_H__

#include "Goknar/Core.h"

class PhysicsContact;

/**
 * The contact resolution routine. One resolver instance
 * can be shared for the whole simulation, as long as you need
 * roughly the same parameters each time (which is normal).
 *
 * @section algorithm Resolution Algorithm
 *
 * The resolver uses an iterative satisfaction algorithm; it loops
 * through each contact and tries to resolve it. Each contact is
 * resolved locally, which may in turn put other contacts in a worse
 * position. The algorithm then revisits other contacts and repeats
 * the process up to a specified iteration limit. It can be proved
 * that given enough iterations, the simulation will get to the
 * correct result. As with all approaches, numerical stability can
 * cause problems that make a correct resolution impossible.
 *
 * @subsection strengths Strengths
 *
 * This algorithm is very fast, much faster than other physics
 * approaches. Even using many more iterations than there are
 * contacts, it will be faster than global approaches.
 *
 * Many global algorithms are unstable under high friction, this
 * approach is very robust indeed for high friction and low
 * restitution values.
 *
 * The algorithm produces visually believable behaviour. Tradeoffs
 * have been made to err on the side of visual floatism rather than
 * computational expense or numerical accuracy.
 *
 * @subsection weaknesses Weaknesses
 *
 * The algorithm does not cope well with situations with many
 * inter-related contacts: stacked boxes, for example. In this
 * case the simulation may appear to jiggle slightly, which often
 * dislodges a box from the stack, allowing it to collapse.
 *
 * Another issue with the resolution mechanism is that resolving
 * one contact may make another contact move sideways against
 * friction, because each contact is handled independently, this
 * friction is not taken into account. If one object is pushing
 * against another, the pushed object may move across its support
 * without friction, even though friction is set between those bodies.
 *
 * In general this resolver is not suitable for stacks of bodies,
 * but is perfect for handling impact, explosive, and flat resting
 * situations.
 */
class GOKNAR_API ContactResolver
{
public:
    /**
     * Creates a new contact resolver with the given number of iterations
     * per resolution call, and optional epsilon values.
     */
    ContactResolver(unsigned int iterations, float velocityEpsilon = 0.01f, float positionEpsilon = 0.01f);

    /**
     * Creates a new contact resolver with the given number of iterations
     * for each kind of resolution, and optional epsilon values.
     */
    ContactResolver(unsigned int velocityIterations, unsigned int positionIterations, float velocityEpsilon = 0.01f, float positionEpsilon = 0.01f);

    /**
     * Returns true if the resolver has valid settings and is ready to go.
     */
    bool IsValid()
    {
        return  (0 < velocityIterations_) &&
                (0 < positionIterations_) &&
                (0.f <= positionEpsilon_) &&
                (0.f <= positionEpsilon_);
    }

    void SetIterations(unsigned int velocityIterations, unsigned int positionIterations)
    {
        velocityIterations_ = velocityIterations;
        positionIterations_ = positionIterations;
    }

    void SetIterations(unsigned int iterations)
    {
        SetIterations(iterations, iterations);
    }

    void SetEpsilon(float velocityEpsilon, float positionEpsilon)
    {
        velocityEpsilon_ = velocityEpsilon;
        positionEpsilon_ = positionEpsilon;
    }

    /**
     * Resolves a set of contacts for both penetration and velocity.
     *
     * Contacts that cannot interact with
     * each other should be passed to separate calls to resolveContacts,
     * as the resolution algorithm takes much longer for lots of
     * contacts than it does for the same number of contacts in small
     * sets.
     *
     * @param contactArray Pointer to an array of contact objects.
     *
     * @param numContacts The number of contacts in the array to resolve.
     *
     * @param numIterations The number of iterations through the
     * resolution algorithm. This should be at least the number of
     * contacts (otherwise some constraints will not be resolved -
     * although sometimes this is not noticable). If the iterations are
     * not needed they will not be used, so adding more iterations may
     * not make any difference. In some cases you would need millions
     * of iterations. Think about the number of iterations as a bound:
     * if you specify a large number, sometimes the algorithm WILL use
     * it, and you may drop lots of frames.
     *
     * @param duration The duration of the previous integration step.
     * This is used to compensate for forces applied.
     */
    void ResolveContacts(PhysicsContact* contactArray, unsigned int numContacts, float duration);

    /**
     * Stores the number of velocity iterations used in the
     * last call to resolve contacts.
     */
    unsigned int velocityIterationsUsed;

    /**
     * Stores the number of position iterations used in the
     * last call to resolve contacts.
     */
    unsigned int positionIterationsUsed;

protected:
    void PrepareContacts(PhysicsContact* contactArray, unsigned int numContacts, float duration);
    void AdjustVelocities(PhysicsContact* contactArray, unsigned int numContacts, float duration);
    void AdjustPositions(PhysicsContact* contacts, unsigned int numContacts, float duration);

    /**
     * Holds the number of iterations to perform when resolving
     * velocity.
     */
    unsigned int velocityIterations_;

    /**
     * Holds the number of iterations to perform when resolving
     * position.
     */
    unsigned int positionIterations_;

    /**
     * To avoid instability velocities smaller
     * than this value are considered to be zero. Too small and the
     * simulation may be unstable, too large and the bodies may
     * interpenetrate visually. A good starting point is the default
     * of 0.01.
     */
    float velocityEpsilon_;

    /**
     * To avoid instability penetrations
     * smaller than this value are considered to be not interpenetrating.
     * Too small and the simulation may be unstable, too large and the
     * bodies may interpenetrate visually. A good starting point is
     * the default of0.01.
     */
    float positionEpsilon_;

private:
    /**
     * Keeps track of whether the internal settings are valid.
     */
    bool validSettings_;
};

#endif