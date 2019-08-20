import bpy
import os

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
    intensity = [0, 0, 0]

class PointLight(Light):
    position = [0, 0, 0]
    
    def Export(self):
        fileBuffer = '\t\t<PointLight id = "' + str(self.id) + '">\n'
        
        # Position
        fileBuffer += '\t\t\t<Position>'
        fileBuffer += str(self.position.x) + ' ' + str(self.position.y) + ' ' + str(self.position.z)
        fileBuffer += '</Position>\n'
        
        # Intensity
        fileBuffer += '\t\t\t<Intensity>'
        fileBuffer += str(self.intensity.x) + ' ' + str(self.intensity.y) + ' ' + str(self.intensity.z)
        fileBuffer += '</Intensity>\n'
        
        fileBuffer += '\t\t</PointLight>\n'
        
        return fileBuffer
        

class DirectionalLight(Light):
    direction = [0, 0, 0]
    
class SpotLight(Light):
    intensity = [0, 0, 0]
    coverageAngle = 0
    falloffAngle = 0
    
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
    textureCoordinates = []
    material = 0
    
    def __init__(self):
        self.vertices = []
        self.faces = []
        self.normals = []
        self.textureCoordinates = []
    
    def Export(self):
        fileBuffer = '\t\t<Mesh id = "' + str(self.id) + '">\n'
        
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
        
        fileBuffer += '\t\t\t<Faces>\n'
        for face in self.faces:
            fileBuffer += '\t\t\t\t' + str(face.x) + ' ' + str(face.y) + ' ' + str(face.z) + '\n'
        fileBuffer += '\t\t\t</Faces>\n'
        
        fileBuffer += '\t\t</Mesh>\n'
        
        return fileBuffer
    
class Scene:
    
    def __init__(self):
        self.cameras = []
        self.lights = []
        self.materials = []
        self.textures = []
        self.meshes = []
    
    cameras = []
    lights = []
    materials = []
    textures = []
    meshes = []
    
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
        for light in self.lights:
            fileBuffer += light.Export()
        fileBuffer += '\t</Lights>\n\n'

        # Materials
        fileBuffer += '\t<Materials>\n'
        for material in self.materials:
            fileBuffer += material.Export()
        fileBuffer += '\t</Materials>\n\n'

        # Meshes
        fileBuffer += '\t<Objects>\n'
        for mesh in self.meshes:
            fileBuffer += mesh.Export()
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
    cameraId = 1
    for obj in scene.objects:
        if(obj.type == 'CAMERA'):
            
            gazeVector = obj.matrix_world.to_quaternion() @ Vector((0.0, 0.0, -1.0))
            upVector = obj.matrix_world.to_quaternion() @ Vector((0.0, 1.0, 0.0))
            
            camera = Camera()
            camera.position = Vector3(obj.location[0], obj.location[1], obj.location[2])
            camera.gaze = Vector3(gazeVector.x, gazeVector.y, gazeVector.z)
            camera.up = Vector3(upVector.x, upVector.y, upVector.z)
            camera.imageName = bpy.path.display_name_from_filepath(fileName) + '_' + str(cameraId) + '.png'
            
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

    pointLightId = 1
    for obj in scene.objects:
        if(obj.type == 'LAMP'):
            
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
                print('Directional Light: ' + obj.name)

            elif(objDataType == 'SPOT'):
                print('Spot Light: ' + obj.name)

    print("Light parsing is done.\n")

    '''
    # Materials
    materialId = 1

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
    
    
    # Meshes
    meshId = 1
    materialId = 1
    for blenderMesh in bpy.data.meshes:
        mesh = Mesh()
        mesh.id = meshId
        
        # Get first face's material as mesh material
        if(len(blenderMesh.materials) > 0):
            blenderMaterial = blenderMesh.materials[blenderMesh.polygons[0].material_index]
            material = Material()
            material.id = materialId

            material.ambient = Vector3(0.2, 0.2, 0.2)

            diffuse = blenderMaterial.node_tree.nodes["Diffuse BSDF"].inputs[0].default_value
            material.diffuse = Vector3(diffuse[0], diffuse[1], diffuse[2])
                
            material.specular = Vector3(0.2, 0.2, 0.2)
            material.phongExponent = 3

            sceneData.materials.append(material)

            mesh.material = materialId
            materialId += 1
            
        for face in blenderMesh.polygons:
            mesh.faces.append(Vector3(face.vertices[0], face.vertices[1], face.vertices[2]))
            
        for vertex in blenderMesh.vertices:
            mesh.vertices.append(Vector3(vertex.co[0], vertex.co[1], vertex.co[2]))
            normal = vertex.normal.to_4d()
            normal.w = 0
            normal = (bpy.data.objects[blenderMesh.name].matrix_world @ normal)
            mesh.normals.append(Vector3(normal.x, normal.y, normal.z))
            
        sceneData.meshes.append(mesh)
        meshId += 1
        
    print("Mesh parsing is done.\n")
    
    sceneData.ExportScene(fileName)
       


if __name__ == '__main__':
    ParseScene()