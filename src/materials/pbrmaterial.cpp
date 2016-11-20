/**
 * @file materials/pbrmaterial.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <materials/pbrmaterial.h>

#include <core/raytracer.h>

// TODO: preallocate upper bound for samples system wide or something
// TODO: maybe interpolate less. there's a cheap way to get position

PBRMaterial::PBRMaterial() {
}

PBRMaterial::~PBRMaterial() {
}

float3 PBRMaterial::f(
    const Vertex & interp,
    const float3 & wo,
    const float3 & wi) const
{
	return 1.0f / (float)M_PI;

    float3 lp = float3(0.0f, 50.0f, 0.0f);
    float2 ls = float2(30.0f, 30.0f);
    float3 ln = float3(0.0f, -1.0f, 0.0f);
    float3 kd = 1.0f / (float)M_PI;
    float3 ld = 1000.0f * ls.x * ls.y;
    float3 n = normalize(interp.normal);

    float3 output;

    #define SAMPLES 1
    float2 jittered[SAMPLES * SAMPLES];
    randJittered2D(SAMPLES, jittered);

    float sampleWeight = 1.0f / (SAMPLES * SAMPLES);
    float pdf = 1.0f / (ls.x * ls.y);

    for (int i = 0; i < SAMPLES * SAMPLES; i++) {
        float3 sample = float3(
            (jittered[i].x - 0.5f) * ls.x,
            0,
            (jittered[i].y - 0.5f) * ls.y) + lp;

        float3 dir = sample - interp.position;
        float dist = length(dir);
        dir /= dist;

        float d1 = saturate(dot(-dir, ln));
        float d2 = saturate(dot(dir, n));

        float3 light = kd * ld * d1 * d2 * pdf * (1.0f / (dist * dist));

        Ray shadowRay(interp.position + dir * 0.001f, dir);

		Collision result;

		//if (!tree->intersect(stack, shadowRay, true, result)) {
			output += light * sampleWeight;
		//}
    }

#if 0
    //if (ray.depth < 2) {
    // TODO: Do we or do we not need to multiply by the cosine
    //    raytracer->emitGlossyReflection(rayBuff, interp.position, interp.normal, ray.direction, ray, float3(0.1f));
    //}

    float k_diff = 0.9f;
    float k_spec = 0.02f;
    float specpow = 80.0f;
    float3 lp = float3(6.0f, 4.0f, -1.5f);
    float lr = 30.0f;

    float3 n = normalize(interp.normal);
    float3 p = interp.position;

    float3 l = lp - p;
    float ld = length(l);
    l /= ld;

    float d = saturate(dot(n, l));
    float falloff = 1.0f - saturate(ld / lr);
    falloff *= falloff;
    d *= falloff;

    float3 r = reflect(l, n);
    float3 v = -ray.direction;

    float s = saturate(dot(r, v));
    s = powf(s, specpow);

    // Pi terms cancel out for punctual light source
    float lighting = (k_diff + s * k_spec * (2 + specpow) / 2.0f) * d;

    float3 output = float3(0, 0, 0);

    if (ray.depth < 3) {
        #define SHADOW_SAMPLES 1
        float sampleWeight = 1.0f / SHADOW_SAMPLES;
        float3 samples[SHADOW_SAMPLES];
        rand3D(SHADOW_SAMPLES, samples);

        for (int i = 0; i < SHADOW_SAMPLES; i++) {
            samples[i] = samples[i] * 0.08f + lp;

            float3 d = samples[i] - p;
            float l = length(d);
            d /= l;

            Ray shadowRay(p + d * .001f, d, ray.weight * lighting * sampleWeight * interp.color.xyz(), l, ray.px, ray.py, ray.depth + 1, Shadow);
            //rayBuff.enqueue(shadowRay);
        }
    }
    //else
        //output += interp.color.xyz() * lighting;
#endif

#if 0
    if (depth < 8) {
        #define BOUNCE_SAMPLES 1
        float sampleWeight = 1.0f / (BOUNCE_SAMPLES * BOUNCE_SAMPLES);

        float2 jittered_2d[BOUNCE_SAMPLES * BOUNCE_SAMPLES];
        float3 bounce_samples[BOUNCE_SAMPLES * BOUNCE_SAMPLES];

        // TODO: Need some kind of divide by maximum distance mode

        randJittered2D(BOUNCE_SAMPLES, jittered_2d);
        mapSamplesCosHemisphere(BOUNCE_SAMPLES, 1.0f, jittered_2d, bounce_samples);
        alignHemisphereNormal(BOUNCE_SAMPLES, bounce_samples, n);

        for (int i = 0; i < BOUNCE_SAMPLES * BOUNCE_SAMPLES; i++) {
            float3 d = bounce_samples[i];
            Ray bounceRay(interp.position + d * .001f, d, ray.weight * sampleWeight * 0.5f, 0.0f, ray.px, ray.py, ray.depth + 1, Shade);
            rayBuff.enqueue(bounceRay);
        }
    }
#endif

#if 0
    float ambient = 0.002f;
    float ao = 0.918f;

    if (ray.depth < 2) {
        #define AO_SAMPLES 4
        float sampleWeight = 1.0f / (AO_SAMPLES * AO_SAMPLES);

        float2 jittered_2d[AO_SAMPLES * AO_SAMPLES];
        float3 ao_samples[AO_SAMPLES * AO_SAMPLES];

        // TODO: Might want some kind of divide by maximum distance mode

        randJittered2D(AO_SAMPLES, jittered_2d);
        mapSamplesCosHemisphere(AO_SAMPLES, 1.0f, jittered_2d, ao_samples);
        alignNormal3D(AO_SAMPLES, ao_samples, n);

        for (int i = 0; i < AO_SAMPLES * AO_SAMPLES; i++) {
            float3 d = ao_samples[i];
            Ray bounceRay(p + d * .001f, d, ray.weight * sampleWeight * ao, 2.0f, ray.px, ray.py, ray.depth + 1, Shadow);
            rayBuff.enqueue(bounceRay);
        }
    }
    else
        output += interp.color.xyz() * ao;

    output += interp.color.xyz() * ambient;

    // TODO: ray.direction and ray are redundant. If this shows up elsewhere, maybe remove the direction
    // argument everywhere.
#endif

    return output;

    //float3 env = raytracer->getAmbientOcclusion(kdStack,
    //    interp.position + triangle->normal * .001f, triangle->normal);
}
