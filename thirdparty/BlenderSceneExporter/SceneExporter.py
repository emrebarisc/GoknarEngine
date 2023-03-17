import bpy
import os
import random

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
    color =[0, 0, 0]

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
        fileBuffer += str(self.color.x) + ' ' + str(self.color.y) + ' ' + str(self.color.z)
        fileBuffer += '</Color>\n'
        
        # Intensity
        fileBuffer += '\t\t\t<Intensity>'
        fileBuffer += str(self.intensity)
        fileBuffer += '</Intensity>\n'
        
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
    name = ""
    
    def __init__(self):
        self.vertices = []
        self.faces = []
        self.normals = []
        self.colors = []
        self.UVs = []
    
    def Export(self):
        fileBuffer = '\t\t<Mesh id = "' + str(self.id) + '">\n'
        
        fileBuffer += '\t\t\t<Name>'
        fileBuffer += self.name
        fileBuffer += '</Name>\n'
        
        fileBuffer += '\t\t\t<Material>' + str(self.material) + '</Material>\n'

        # Vertex Data
        fileBuffer += '\t\t\t<Vertices>\n'
        for vertex in self.vertices:
            fileBuffer += '\t\t\t\t' + str(vertex.x) + ' ' + str(vertex.y) + ' ' + str(vertex.z) + '\n'
        fileBuffer += '\t\t\t</Vertices>\n'
        
        fileBuffer += '\t\t\t<Normals>\n'
        for normal in self.normals:
            fileBuffer += '\t\t\t\t' + str(normal.x) + ' ' + str(normal.y) + ' ' + str(normal.z) + '\n'
        fileBuffer += '\t\t\t</Normals>\n'
        
        fileBuffer += '\t\t\t<UVs>\n'
        for textureCoordinate in self.UVs:
            fileBuffer += '\t\t\t\t' + str(textureCoordinate.x) + ' ' + str(textureCoordinate.y) + '\n'
        fileBuffer += '\t\t\t</UVs>\n'
        
        fileBuffer += '\t\t\t<Faces>\n'
        for face in self.faces:
            fileBuffer += '\t\t\t\t' + str(face.x) + ' ' + str(face.y) + ' ' + str(face.z) + '\n'
        fileBuffer += '\t\t\t</Faces>\n'
        
        fileBuffer += '\t\t</Mesh>\n'
        
        return fileBuffer


class MeshObject:
    pivotPoint = Vector3(0, 0, 0)
    relativeLocation = Vector3(0, 0, 0)
    relativeRotation = Vector3(0, 0, 0)
    relativeScaling = Vector3(1, 1, 1)
    worldLocation = Vector3(0, 0, 0)
    worldRotation = Vector3(0, 0, 0)
    worldScaling = Vector3(1, 1, 1)
    parentMesh = 0
    name = ""
    
    def __init__(self):
        pivotPoint = Vector3(0, 0, 0)
        self.relativeLocation = Vector3(0, 0, 0)
        self.relativeRotation = Vector3(0, 0, 0)
        self.relativeScaling = Vector3(1, 1, 1)
        self.worldLocation = Vector3(0, 0, 0)
        self.worldRotation = Vector3(0, 0, 0)
        self.worldScaling = Vector3(1, 1, 1)
    
    def Export(self):
        fileBuffer = '\t\t<MeshObject id = "' + str(self.id) + '">\n'
        
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
        
        fileBuffer += '\t\t\t<Mesh>' + str(self.parentMesh) + '</Mesh>\n'
        
        fileBuffer += '\t\t</MeshObject>\n'
        
        return fileBuffer
    
class Scene:
    def __init__(self):
        self.cameras = []
        self.lights = []
        self.materials = []
        self.textures = []
        self.meshes = []
        self.objects = []
    
    cameras = []
    lights = []
    materials = []
    textures = []
    meshes = []
    objects = []
    
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
    fileName = os.path.splitext(bpy.data.filepath)[0] + ".xml"

    scene = bpy.context.scene

    sceneData = Scene()
    sceneData.bgColor = Vector3(255 - scene.render.stamp_foreground[0] * 255, 255 - scene.render.stamp_foreground[1] * 255, 255 - scene.render.stamp_foreground[2] * 255)

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
    for obj in bpy.data.objects:
        if(obj.type == 'LIGHT'):
            
            objDataType = getattr(obj.data, 'type', '')
            
            light = None
            
            # Point Light
            if(objDataType == 'POINT'):
                light = PointLight()
               
                light.id = pointLightId
                light.position = Vector3(obj.location[0], obj.location[1], obj.location[2])
                
                # Intensity
                intensityColor = obj.data.node_tree.nodes["Emission"].inputs[0].default_value
                intensityValue = obj.data.node_tree.nodes["Emission"].inputs[1].default_value
                intensity = [intensityColor[0] * intensityValue, intensityColor[1] * intensityValue, intensityColor[2] * intensityValue]
                
                light.intensity = Vector3(intensity[0], intensity[1], intensity[2])
            
                pointLightId += 1
                
                sceneData.lights.append(light)
                
            elif(objDataType == 'SUN'):
                light = DirectionalLight()
               
                light.id = directionalLightId
                
                # Intensity
                intensityColor = obj.data.color
                intensityValue = obj.data.node_tree.nodes["Emission"].inputs["Strength"].default_value
                intensity = [intensityColor[0] * intensityValue, intensityColor[1] * intensityValue, intensityColor[2] * intensityValue]

                direction = obj.angle
                
                light.intensity = intensityValue
                light.color = Vector3(intensityColor[0], intensityColor[1], intensityColor[2])
                light.direction = Vector3(direction[0], direction[1], direction[2])
            
                directionalLightId += 1
                
                sceneData.lights.append(light)
                print('Directional Light: ' + obj.name)

            elif(objDataType == 'SPOT'):
                print('Spot Light: ' + obj.name)

    print("Light parsing is done.\n")

    '''
    # Materials
    materialId = 0

    for blenderMaterial in bpy.data.materials:
        material = Material()
        material.id = materialId
        
        material.ambient = Vector3(0.2, 0.2, 0.2)
        
        diffuse = blenderMaterial.node_tree.nodes["Diffuse BSDF"].inputs[0].default_value
        material.diffuse = Vector3(diffuse[0], diffuse[1], diffuse[2])
            
        material.specular = Vector3(0.2, 0.2, 0.2)
        material.phongExponent = 3
        
        sceneData.materials.append(material)
        
        materialId += 1
    
    print("Material parsing is done.\n")
    '''
#    # Meshes
#    meshId = 0
#    materialId = 0
#    for blenderMesh in bpy.data.meshes:
#        mesh = Mesh()
#        mesh.id = meshId
#        
#        # Get first face's material as mesh material
#        if(len(blenderMesh.materials) > 0):
#            blenderMaterial = blenderMesh.materials[blenderMesh.polygons[0].material_index]
#            material = Material()
#            material.id = materialId

#            material.ambient = Vector3(0.2, 0.2, 0.2)
#            
#            if(blenderMaterial != None and blenderMaterial.node_tree != None):
#                nodes = blenderMaterial.node_tree.nodes
#                principled = next(n for n in nodes if n.type == 'BSDF_PRINCIPLED')
#                baseColor = principled.inputs['Base Color']
#                diffuse = baseColor.default_value
#                material.diffuse = Vector3(diffuse[0], diffuse[1], diffuse[2])  
#                
#                material.specular = Vector3(0.2, 0.2, 0.2)
#                material.phongExponent = 3
#                sceneData.materials.append(material)
#                mesh.material = materialId
#                materialId += 1
#            
#        #uv_layer = blenderMesh.uv_layers.active.data
#         
#        for vertex in blenderMesh.vertices:
#            mesh.vertices.append(Vector3(vertex.co[0], vertex.co[1], vertex.co[2]))
#            normal = vertex.normal.to_4d()
#            normal.w = 0
#            
#            normal = (bpy.data.objects[blenderMesh.name].matrix_world @ normal)
#            mesh.normals.append(Vector3(normal.x, normal.y, normal.z))
#            mesh.UVs.append(Vector2(0, 0))
#                        
#        for face in blenderMesh.polygons:
#            mesh.faces.append(Vector3(face.vertices[0], face.vertices[1], face.vertices[2]))
#            
#            #for loop_index in range(face.loop_start, face.loop_start + face.loop_total):
#                #print(loop_index)
#                #mesh.UVs[loop_index] = Vector2(uv_layer[loop_index].uv.x, uv_layer[loop_index].uv.y)
#            
##        mesh.relativeLocation = blenderMesh.location
##        mesh.relativeRotation = blenderMesh.rotation_eular
##        mesh.relativeScaling = blenderMesh.scale
#        
#        mesh.name = blenderMesh.name    
#            
#        sceneData.meshes.append(mesh)
#        meshId += 1
#    
    # Objects
    objectId = 0
    for blenderObject in bpy.data.objects:
        meshObject = MeshObject()
        meshObject.id = objectId
        
        if("Camera" in blenderObject.name):
             continue
        
        if("Terrain" not in blenderObject.name):
            meshObject.relativeLocation = Vector3(0, 0, 0)
            meshObject.relativeRotation = Vector3(0, 0, 0)
            meshObject.relativeScaling = Vector3(1, 1, 1)
            
            meshObject.worldLocation = blenderObject.location
            meshObject.worldRotation = blenderObject.rotation_euler
            meshObject.worldScaling = blenderObject.scale
            
            randomRotationX = random.randrange(0, 3, 1) / 10
            randomRotationY = random.randrange(0, 3, 1) / 10
            randomRotationZ = random.randrange(0, 359, 1)
            meshObject.worldRotation = Vector3(randomRotationX, randomRotationY, randomRotationZ)
            
            randomScale = random.randrange(75, 125, 1) / 100
            meshObject.worldScaling = Vector3(randomScale, randomScale, randomScale)
        
        meshObject.name = blenderObject.name    
        sceneData.objects.append(meshObject)
        objectId += 1
        
    print("Object parsing is done.\n")
        
    for object in sceneData.objects:
        objectName = object.name
        splittedName = objectName.split(".")
        
        name = splittedName[0]
        number = ""
        
        if(len(splittedName) > 1) :
             number = splittedName[1]
        
        if("TreeBody" in name):
            for subObject in sceneData.objects:
                objectName = subObject.name
                splittedName = objectName.split(".")
                subName = splittedName[0]
                subNumber = ""
                
                if(len(splittedName) > 1) :
                     subNumber = splittedName[1]
                
                if(subName == "TreeBrunches" and subNumber == number):
                     subObject.worldLocation = object.worldLocation
                     subObject.worldRotation = object.worldRotation
                     subObject.worldScaling = object.worldScaling
                     break
    
    sceneData.ExportScene(fileName)
       


if __name__ == '__main__':
    '''agacGovdesi = None
    agacYapraklari = None
    
    for o in bpy.context.scene.objects:
        if("AGAC_GOVDESI" in o.name) :
            agacGovdesi = o
        elif("AGAC_YAPRAKLARI" in o.name) :
            agacYapraklari = o
            
    
    for o in bpy.context.scene.objects:
        if("TreeBody" in o.name) :
            newObject = agacGovdesi.copy()
            newObject.location = o.location
            bpy.context.scene.collection.objects.link(newObject)
            o.select_set(True)
            bpy.ops.object.delete()
        elif("TreeBrunches" in o.name) :
            newObject = agacYapraklari.copy()
            newObject.location = o.location
            bpy.context.scene.collection.objects.link(newObject)
            o.select_set(True)
            bpy.ops.object.delete()'''
            
            
    
    ParseScene()