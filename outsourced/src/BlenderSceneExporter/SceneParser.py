import bpy
import os

from mathutils import Vector

from SceneData import *

# Write to an XML file named same with the blender file

def ParseScene():
    fileName = os.path.splitext(bpy.data.filepath)[0] + ".xml"

    scene = bpy.context.scene

    sceneData = Scene()
    sceneData.bgColor = Vector3(255 - scene.render.stamp_foreground[0] * 255, 255 - scene.render.stamp_foreground[1] * 255, 255 - scene.render.stamp_foreground[2] * 255)

    #Cameras
    cameraId = 1
    for obj in scene.objects:
        if(obj.type == 'CAMERA'):
            
            gazeVector = obj.matrix_world.to_quaternion() * Vector((0.0, 0.0, -1.0))
            upVector = obj.matrix_world.to_quaternion() * Vector((0.0, 1.0, 0.0))
            
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
            
            camera.numOfSamples = bpy.context.scene.render.antialiasing_samples
            
            camera.fovY = obj.data.angle_y / 3.01675 * 180
            
            cameraForwardVector = obj.matrix_world.to_quaternion() * Vector((0.0, 0.0, -1.0)) * camera.nearDistance
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
            
        sceneData.meshes.append(mesh)
        meshId += 1
        
    print("Mesh parsing is done.\n")
    
    sceneData.ExportScene(fileName)
    
if __name__ == '__main__':
    ParseScene()
    
    print(bpy.context.scene.render)