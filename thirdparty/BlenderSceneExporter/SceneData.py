import bpy
import bpy.ops
import bmesh
import os
import math
import sys

from mathutils import Vector

from enum import Enum

class Vector2:        
    def __init__(self, _x, _y):
        self.x = _x
        self.y = _y
    
    x = 0
    y = 0

class Vector3:        
    def __init__(self, _x, _y, _z):
        self.x = _x
        self.y = _y
        self.z = _z
    
    x = 0
    y = 0
    z = 0

class Vector4:        
    def __init__(self, _x, _y, _z, _w):
        self.x = _x
        self.y = _y
        self.z = _z
        self.w = _w
    
    x = 0
    y = 0
    z = 0
    w = 0

class Camera:
    id = 0
    
    nearPlane = Vector4(-0.5, 0.5, -0.5, 0.5)
    nearDistance = 1
    imageResolution = Vector2(0, 0)
    
    gaze = Vector3(0, 0, 0)
    up = Vector3(0, 0, 0)
    position = Vector3(0, 0, 0)
    
    fovY = 0
    
    gazePoint = Vector3(0, 0, 0)
    
    numOfSamples = 1
    
    imageName = ""
    
    def __init__(self):
        self.id = 0
    
        self.nearPlane = Vector4(-0.5, 0.5, -0.5, 0.5)
        self.nearDistance = 1
        self.imageResolution = Vector2(0, 0)
        
        self.gaze = Vector3(0, 0, 0)
        self.up = Vector3(0, 0, 0)
        self.position = Vector3(0, 0, 0)
        
        self.numOfSamples = 1
        
        self.imageName = ""
        
    def SetNearPlaneWithRatio(self, ratio):
        nearPlaneVertical = 0.5 / ratio
            
        self.nearPlane = Vector4(-0.5, 0.5, -nearPlaneVertical, nearPlaneVertical)
    
    def Export(self):
        fileBuffer = '\t\t<Camera id = "' + str(self.id) + '">\n'
        
        # Simple camera
        #fileBuffer = '\t\t<Camera id =:"' + str(self.id) + '" type = "simple">\n'
        
        # fileBuffer += '\t\t\t<FovY>' + str(self.fovY) + '</FovY>\n'
        
        # fileBuffer += '\t\t\t<GazePoint>' 
        # fileBuffer += str(self.gazePoint.x) + ' ' + str(self.gazePoint.y) + ' ' + str(self.gazePoint.z) 
        # fileBuffer += '</GazePoint> \n'
        
        # Camera position
        fileBuffer += '\t\t\t<Position>'
        fileBuffer += str(self.position.x) + ' ' + str(self.position.y) + ' ' + str(self.position.z)
        fileBuffer += '</Position>\n'
        
        # Camera gaze vector
        fileBuffer += '\t\t\t<Gaze>'
        fileBuffer += str(self.gaze.x) + ' ' + str(self.gaze.y) + ' ' + str(self.gaze.z)
        fileBuffer += '</Gaze>\n'
        
        # Camera up vector
        fileBuffer += '\t\t\t<Up>'
        fileBuffer += str(self.up.x) + ' ' + str(self.up.y) + ' ' + str(self.up.z)
        fileBuffer += '</Up>\n'
        
        # Near plane is only for not simple cameras
        fileBuffer += '\t\t\t<NearPlane>' + str(self.nearPlane.x) + ' ' + str(self.nearPlane.y) + ' ' + str(self.nearPlane.z) + ' ' + str(self.nearPlane.w) + '</NearPlane>\n'
        
        fileBuffer += '\t\t\t<NearDistance>' + str(self.nearDistance) + '</NearDistance>\n'
        
        fileBuffer += '\t\t\t<ImageResolution>' + str(self.imageResolution.x) + ' ' + str(self.imageResolution.y) + '</ImageResolution>\n'
        
        fileBuffer += '\t\t\t<NumSamples>' + str(self.numOfSamples) + '</NumSamples>\n'
        
        fileBuffer += '\t\t\t<ImageName>' + self.imageName + '</ImageName>\n'
        
        fileBuffer += '\t\t</Camera>\n'
        
        return fileBuffer

class Light:
    id = 0
    intensity = 0
    radius = 0
    color = [0, 0, 0]

class PointLight(Light):
    position = [0, 0, 0]
    
    def Export(self):
        fileBuffer = '\t\t<PointLight id = "' + str(self.id) + '">\n'
        
        # Position
        fileBuffer += '\t\t\t<Position>'
        fileBuffer += str(self.position.x) + ' ' + str(self.position.y) + ' ' + str(self.position.z)
        fileBuffer += '</Position>\n'
        
        # Position
        fileBuffer += '\t\t\t<Color>'
        fileBuffer += str(self.color[0]) + ' ' + str(self.color[1]) + ' ' + str(self.color[2])
        fileBuffer += '</Color>\n'
        
        # Intensity
        fileBuffer += '\t\t\t<Intensity>'
        fileBuffer += str(self.intensity)
        fileBuffer += '</Intensity>\n'
        
        # Intensity
        fileBuffer += '\t\t\t<Radius>'
        fileBuffer += str(self.radius)
        fileBuffer += '</Radius>\n'
        
        fileBuffer += '\t\t</PointLight>\n'
        
        return fileBuffer

class DirectionalLight(Light):
    direction = [0, 0, 0]
    
    def Export(self):
        fileBuffer = '\t\t<DirectionalLight id = "' + str(self.id) + '">\n'
        
        # Position
        fileBuffer += '\t\t\t<Direction>'
        fileBuffer += str(self.direction.x) + ' ' + str(self.direction.y) + ' ' + str(self.direction.z)
        fileBuffer += '</Direction>\n'
        
        # Position
        fileBuffer += '\t\t\t<Color>'
        fileBuffer += str(self.color.x) + ' ' + str(self.color.y) + ' ' + str(self.color.z)
        fileBuffer += '</Color>\n'
        
        # Intensity
        fileBuffer += '\t\t\t<Intensity>'
        fileBuffer += str(self.intensity)
        fileBuffer += '</Intensity>\n'
        
        fileBuffer += '\t\t</DirectionalLight>\n'
        
        return fileBuffer
    
class SpotLight(Light):
    position = [0, 0, 0]
    coverageAngle = 0
    falloffAngle = 0
    
    def Export(self):
        fileBuffer = '\t\t<SpotLight id = "' + str(self.id) + '">\n'
        
        # Direction
        fileBuffer += '\t\t\t<Direction>'
        fileBuffer += str(self.direction.x) + ' ' + str(self.direction.y) + ' ' + str(self.direction.z)
        fileBuffer += '</Direction>\n'
        
        # Position
        fileBuffer += '\t\t\t<Position>'
        fileBuffer += str(self.position.x) + ' ' + str(self.position.y) + ' ' + str(self.position.z)
        fileBuffer += '</Position>\n'
        
        # Color
        fileBuffer += '\t\t\t<Color>'
        fileBuffer += str(self.color.x) + ' ' + str(self.color.y) + ' ' + str(self.color.z)
        fileBuffer += '</Color>\n'
        
        # Intensity
        fileBuffer += '\t\t\t<Intensity>'
        fileBuffer += str(self.intensity)
        fileBuffer += '</Intensity>\n'
        
        # Coverage Angle
        fileBuffer += '\t\t\t<CoverageAngle>'
        fileBuffer += str(self.coverageAngle.x) + ' ' + str(self.coverageAngle.y) + ' ' + str(self.coverageAngle.z)
        fileBuffer += '</CoverageAngle>\n'
        
        # Falloff Angle
        fileBuffer += '\t\t\t<FalloffAngle>'
        fileBuffer += str(self.falloffAngle.x) + ' ' + str(self.falloffAngle.y) + ' ' + str(self.falloffAngle.z)
        fileBuffer += '</FalloffAngle>\n'
        
        fileBuffer += '\t\t</SpotLight>\n'
        
        return fileBuffer
    
class Material:
    id = 0
    ambient = Vector3(0, 0, 0)
    diffuse = Vector3(0, 0, 0)
    mirror = Vector3(0, 0, 0)
    transparency = Vector3(0, 0, 0)
    specular = Vector3(0, 0, 0)
    phongExponent = 3
    
    def Export(self):
        fileBuffer = '\t\t<Material id = "' + str(self.id) + '">\n'
        
        fileBuffer += '\t\t\t<AmbientReflectance>'
        fileBuffer += str(self.ambient.x) + ' ' + str(self.ambient.y) + ' ' + str(self.ambient.z)
        fileBuffer += '</AmbientReflectance>\n'
        
        fileBuffer += '\t\t\t<DiffuseReflectance>'
        fileBuffer += str(self.diffuse.x) + ' ' + str(self.diffuse.y) + ' ' + str(self.diffuse.z)
        fileBuffer += '</DiffuseReflectance>\n'
        
        # TODO
        fileBuffer += '\t\t\t<SpecularReflectance>'
        fileBuffer += str(self.specular.x) + ' ' + str(self.specular.y) + ' ' + str(self.specular.z)
        fileBuffer += '</SpecularReflectance>\n'
        
        fileBuffer += '\t\t\t<PhongExponent>' + str(self.phongExponent) + '</PhongExponent>\n'
        
        fileBuffer += '\t\t</Material>\n'
        
        return fileBuffer
    
class Mesh:
    id = 0
    vertices = []
    faces = []
    normals = []
    colors = []
    UVs = []
    material = 0
    relativeLocation = None
    relativeRotation = None
    relativeScaling = None
    name = ""
    
    def __init__(self):
        self.vertices = []
        self.faces = []
        self.normals = []
        self.colors = []
        self.UVs = []
        self.relativeLocation = None
        self.relativeRotation = None
        self.relativeScaling = None
        self.name = ""
    
    def Export(self):
        fileBuffer = '\t\t<StaticMesh id = "' + str(self.id) + '">\n'
        
        
        if self.name:
            
            fileBuffer += '\t\t\t<Name>'
            fileBuffer += self.name
            fileBuffer += '</Name>\n'
            
            fileBuffer += '\t\t\t<Path>'
            fileBuffer += 'Meshes/' + 'SM_' + self.name + '.fbx'
            fileBuffer += '</Path>\n'
        
        if self.material:
            fileBuffer += '\t\t\t<Material>' + str(self.material) + '</Material>\n'

        if(self.relativeLocation != None):
            fileBuffer += '\t\t\t<RelativeLocation>' + str(self.relativeLocation.x) + ' ' + str(self.relativeLocation.y) + ' ' + str(self.relativeLocation.z) + '</RelativeLocation>'

        if(self.relativeRotation != None):
            fileBuffer += '\t\t\t<RelativeRotation>' + str(self.relativeRotation.x) + ' ' + str(self.relativeRotation.y) + ' ' + str(self.relativeRotation.z) + '</RelativeRotation>'

        if(self.relativeScaling != None):
            fileBuffer += '\t\t\t<RelativeScaling>' + str(self.relativeScaling.x) + ' ' + str(self.relativeScaling.y) + ' ' + str(self.relativeScaling.z) + '</RelativeScaling>'
        
        # Vertex Data
        if self.vertices:
            fileBuffer += '\t\t\t<Vertices>\n'
            for vertex in self.vertices:
                fileBuffer += '\t\t\t\t' + str(vertex.x) + ' ' + str(vertex.y) + ' ' + str(vertex.z) + '\n'
            fileBuffer += '\t\t\t</Vertices>\n'
        
        if self.normals:
            fileBuffer += '\t\t\t<Normals>\n'
            for normal in self.normals:
                fileBuffer += '\t\t\t\t' + str(normal.x) + ' ' + str(normal.y) + ' ' + str(normal.z) + '\n'
            fileBuffer += '\t\t\t</Normals>\n'
        
        if self.UVs:
            fileBuffer += '\t\t\t<UVs>\n'
            for textureCoordinate in self.UVs:
                fileBuffer += '\t\t\t\t' + str(textureCoordinate.x) + ' ' + str(textureCoordinate.y) + '\n'
            fileBuffer += '\t\t\t</UVs>\n'
        
        if self.faces:
            fileBuffer += '\t\t\t<Faces>\n'
            for face in self.faces:
                fileBuffer += '\t\t\t\t' + str(face.x) + ' ' + str(face.y) + ' ' + str(face.z) + '\n'
            fileBuffer += '\t\t\t</Faces>\n'
        
        fileBuffer += '\t\t</StaticMesh>\n'
        
        return fileBuffer
    
class Object:
    name = ""
    id = -1
    linkedMeshId = -1
    pivotPoint = Vector3(0, 0, 0)
    relativeLocation = Vector3(0, 0, 0)
    relativeRotation = Vector3(0, 0, 0)
    relativeScaling = Vector3(1, 1, 1)
    worldLocation = Vector3(0, 0, 0)
    worldRotation = Vector3(0, 0, 0)
    worldScaling = Vector3(1, 1, 1)
    
    def __init__(self):
        self.name = []
        self.id = -1
        self.linkedMeshId = -1
        self.pivotPoint = Vector3(0, 0, 0)
        self.relativeLocation = Vector3(0, 0, 0)
        self.relativeRotation = Vector3(0, 0, 0)
        self.relativeScaling = Vector3(1, 1, 1)
        self.worldLocation = Vector3(0, 0, 0)
        self.worldRotation = Vector3(0, 0, 0)
        self.worldScaling = Vector3(1, 1, 1)
    
    def Export(self):
        
        fileBuffer = '\t\t<StaticMeshObject id = "' + str(self.id) + '">\n'
        
        fileBuffer += '\t\t\t<Mesh>' + str(self.linkedMeshId) + '</Mesh>\n'
        
        fileBuffer += '\t\t\t<Name>'
        fileBuffer += self.name
        fileBuffer += '</Name>\n'
        
        fileBuffer += '\t\t\t<PivotPoint>' + str(self.pivotPoint.x) + ' ' + str(self.pivotPoint.y) + ' ' + str(self.pivotPoint.z) + '</PivotPoint>\n'
             
        fileBuffer += '\t\t\t<RelativePosition>' + str(self.relativeLocation.x) + ' ' + str(self.relativeLocation.y) + ' ' + str(self.relativeLocation.z) + '</RelativePosition>\n'
        fileBuffer += '\t\t\t<RelativeRotation>' + str(self.relativeRotation.x) + ' ' + str(self.relativeRotation.y) + ' ' + str(self.relativeRotation.z) + '</RelativeRotation>\n'
        fileBuffer += '\t\t\t<RelativeScaling>' + str(self.relativeScaling.x) + ' ' + str(self.relativeScaling.y) + ' ' + str(self.relativeScaling.z) + '</RelativeScaling>\n'
        
        fileBuffer += '\t\t\t<WorldPosition>' + str(self.worldLocation.x) + ' ' + str(self.worldLocation.y) + ' ' + str(self.worldLocation.z) + '</WorldPosition>\n'
        fileBuffer += '\t\t\t<WorldRotation>' + str(self.worldRotation.x) + ' ' + str(self.worldRotation.y) + ' ' + str(self.worldRotation.z) + '</WorldRotation>\n'
        fileBuffer += '\t\t\t<WorldScaling>' + str(self.worldScaling.x) + ' ' + str(self.worldScaling.y) + ' ' + str(self.worldScaling.z) + '</WorldScaling>\n'
        
        fileBuffer += '\t\t</StaticMeshObject>\n\n'
        
        return fileBuffer
    
class Scene:
    def __init__(self):
        self.cameras = []
        self.lights = []
        self.materials = []
        self.textures = []
        self.meshes = []
        self.objects = []
        self.meshMap = {}
        self.meshIdMap = {}
    
    cameras = []
    lights = []
    materials = []
    textures = []
    meshes = []
    objects = []
    meshMap = {}
    meshIdMap = {}
    
    ambientLight = Vector3(100, 100, 100)
    bgColor = Vector3(0, 0, 0)
        
    def ExportScene(self, filePath):
        fileBuffer = '<Scene>\n'
        
        fileBuffer += '\t<BackgroundColor>' + str(int(self.bgColor.x)) + ' ' + str(int(self.bgColor.y)) + ' ' + str(int(self.bgColor.z)) + '</BackgroundColor>\n'

        # Cameras
        fileBuffer += '\t<Cameras>\n'
        for camera in self.cameras:
            fileBuffer += camera.Export()
        fileBuffer += '\t</Cameras>\n\n'

        # Lights
        fileBuffer += '\t<Lights>\n'
        fileBuffer += '\t\t<AmbientLight>' + str(self.ambientLight.x) + ' ' + str(self.ambientLight.y) + ' ' + str(self.ambientLight.z) + '</AmbientLight>\n'
        
        print(self.lights)
        for light in self.lights:
            fileBuffer += light.Export()
        fileBuffer += '\t</Lights>\n\n'

        # Materials
        fileBuffer += '\t<Materials>\n'
        for material in self.materials:
            fileBuffer += material.Export()
        fileBuffer += '\t</Materials>\n\n'

        # Meshes
        fileBuffer += '\t<Meshes>\n'
        for mesh in self.meshes:
            fileBuffer += mesh.Export()
        fileBuffer += '\t</Meshes>\n\n'
        
        # Objects
        fileBuffer += '\t<Objects>\n'
        for object in self.objects:
            fileBuffer += object.Export()
        fileBuffer += '\t</Objects>\n\n'

        fileBuffer += '</Scene>'

        with open(filePath, 'w') as file:
            file.write(fileBuffer)

def ParseScene():
    scene = bpy.context.scene

    sceneData = Scene()
    sceneData.bgColor = Vector3(255 - scene.render.stamp_foreground[0] * 255, 255 - scene.render.stamp_foreground[1] * 255, 255 - scene.render.stamp_foreground[2] * 255)

    absolutePath = bpy.path.abspath("//")

    #Cameras
    cameraId = 0
    for obj in scene.objects:
        if(obj.type == 'CAMERA'):
            gazeVector = obj.matrix_world.to_quaternion() @ Vector((0.0, 0.0, -1.0))
            upVector = obj.matrix_world.to_quaternion() @ Vector((0.0, 1.0, 0.0))
            
            camera = Camera()
            camera.position = Vector3(obj.location[0], obj.location[1], obj.location[2])
            camera.gaze = Vector3(gazeVector.x, gazeVector.y, gazeVector.z)
            camera.up = Vector3(upVector.x, upVector.y, upVector.z)
            
            camera.imageResolution = Vector2(int(scene.render.resolution_x * scene.render.resolution_percentage / 100), int(scene.render.resolution_y * scene.render.resolution_percentage / 100))
            
            ratio = scene.render.resolution_x / scene.render.resolution_y
            camera.SetNearPlaneWithRatio(ratio)
            
            #camera.nearPlane = Vector4(scene.render.border_min_x, scene.render.border_max_x, scene.render.border_min_y, scene.render.border_max_y)
            #camera.nearDistance = obj.data.clip_start
            
            camera.numOfSamples = 1
            
            camera.fovY = obj.data.angle_y / 3.01675 * 180
            
            cameraForwardVector = obj.matrix_world.to_quaternion() @ Vector((0.0, 0.0, -1.0)) * camera.nearDistance
            camera.gazePoint = Vector3(camera.position.x + cameraForwardVector[0], camera.position.y + cameraForwardVector[1], camera.position.z + cameraForwardVector[2])
            
            camera.id = cameraId
            cameraId += 1
            
            sceneData.cameras.append(camera)
            
    print("Camera parsing is done.\n")

    # Lights

    pointLightId = 0
    directionalLightId = 0
    spotLightId = 0
    
    meshId = 0
    
    objectId = 0    
    for obj in scene.objects:
        if(obj.type == 'LIGHT'):
            
            objDataType = getattr(obj.data, 'type', '')
            
            light = None
            
            # Point Light
            if(objDataType == 'POINT'):
                light = PointLight()
               
                light.id = pointLightId
                light.position = Vector3(obj.location[0], obj.location[1], obj.location[2])
                
                # Intensity
                light.color = obj.data.color
                intensityValue = obj.data.energy
                intensity = [light.color[0] * intensityValue, light.color[1] * intensityValue, light.color[2] * intensityValue]
                
                light.intensity = obj.data.energy
                
                light.radius = obj.data.distance
            
                pointLightId += 1
                
                sceneData.lights.append(light)
                
            elif(objDataType == 'SUN'):
                light = DirectionalLight()
               
                light.id = directionalLightId
                
                # Intensity
                intensityColor = obj.data.color
                intensityValue = obj.data.energy

                direction = obj.angle
                
                light.intensity = intensityValue
                light.color = Vector3(intensityColor[0], intensityColor[1], intensityColor[2])
                light.direction = Vector3(direction[0], direction[1], direction[2])
            
                directionalLightId += 1
                
                sceneData.lights.append(light)
                print('Directional Light: ' + obj.name)

            elif(objDataType == 'SPOT'):
                print('Spot Light: ' + obj.name)
        if(obj.type == 'MESH'):
            objectData = obj.data
            if objectData.id_data:
                object = Object()
                object.name = objectData.name
                object.id = objectId
                object.worldLocation = Vector3(obj.location[0], obj.location[1], obj.location[2])
                
                rotationInDegrees = Vector3(obj.rotation_euler[0] * 180.0 / math.pi, obj.rotation_euler[1] * 180.0 / math.pi, obj.rotation_euler[2] * 180.0 / math.pi)
                object.worldRotation = rotationInDegrees
                
                object.worldScaling = Vector3(obj.scale[0], obj.scale[1], obj.scale[2])
                
                sceneData.objects.append(object)
                
                if obj.data.id_data.name not in sceneData.meshMap:
                    sceneData.meshMap[obj.data.id_data.name] = obj
                    sceneData.meshIdMap[obj.data.id_data.name] = meshId
                    object.linkedMeshId = meshId
                    
                    meshId += 1
                else:
                    object.linkedMeshId = sceneData.meshIdMap[obj.data.id_data.name]
                    
                objectId += 1
                        
    # Meshes
    for meshName in sceneData.meshMap:
        object = sceneData.meshMap[meshName]
        
        exportMesh = Mesh()
        exportMesh.id = sceneData.meshIdMap[meshName]
        exportMesh.name = meshName
        
        bm = bmesh.new()
        bm.from_mesh(object.data)
        
        bmesh.ops.triangulate(bm, faces=bm.faces[:], quad_method='BEAUTY', ngon_method='BEAUTY')

        object.select_set(True)

        exportPath = absolutePath + 'Content\\Meshes\\SM_' + meshName + '.fbx'
        bpy.ops.export_scene.fbx(
            filepath=exportPath, 
            check_existing=False, 
            use_selection=True,
            object_types={'ARMATURE', 'EMPTY', 'MESH', 'OTHER'}, 
            apply_unit_scale=True, 
            apply_scale_options='FBX_SCALE_ALL',
            path_mode='COPY', 
            embed_textures=False, 
            axis_forward='X', 
            axis_up='Z')
            
        object.select_set(False)
                    
        sceneData.meshes.append(exportMesh)
    
    fileName = bpy.context.scene.name
    xmlPath = absolutePath + 'Content\\Scenes\\' + fileName + '.xml'
    sceneData.ExportScene(xmlPath)
    print("Exported to: " + xmlPath + "\n")
    
if __name__ == '__main__':
    ParseScene()
