    for obj in scene.objects:
        print(obj.type)
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
                print('Directional Light: ' + obj.name)

            elif(objDataType == 'SPOT'):
                print('Spot Light: ' + obj.name)

    print("Light parsing is done.\n")
