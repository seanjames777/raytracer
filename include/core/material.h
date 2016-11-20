//
//  material.h
//  metalrt
//
//  Created by Sean James on 11/11/16.
//  Copyright © 2016 Apple. All rights reserved.
//

#ifndef material_h
#define material_h

struct Material {
    float3 diffuse;
    float3 specular;
    float  specularPower;
    float3 ambient;
    float  reflectivity;
};

#endif /* material_h */
