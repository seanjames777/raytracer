/**
 * @file core/raytracer.cpp
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#include <core/raytracer.h>

#include <math/matrix.h>
#include <materials/pbrmaterial.h>
#include <util/imageloader.h>
#include <map>

#include <iostream>
#include <cassert>

// TODO: thread affinity

// TODO: Increasing tile size seems to make the computer happy
#define BLOCKW 32
#define BLOCKH 32

// TODO: Come up with a better workflow

// TODO: Might be better to compact textures to RGB8
std::map<std::string, Image<float, 4> *> textures;

Image<float, 4> *loadTexture(std::string name) {
    std::string filename = relToExeDir("content/textures/" + name);
    std::cout << "Load texture " << filename << std::endl;

    if (textures.find(filename) != textures.end())
        return textures[filename];

    auto image = ImageLoader::load(filename);
    textures[filename] = image;

    return image;
}

Vertex transformVertex(const Vertex & vertex, const float4x4 & transform,
    const float4x4 & transformInverseTranspose)
{
    float4 position = transform * float4(vertex.position, 1.0f);
    float4 normal = transformInverseTranspose * float4(vertex.normal, 0.0f);
    float4 tangent = transformInverseTranspose * float4(vertex.tangent, 0.0f);

    return Vertex(position.xyz(), normalize(normal.xyz()), normalize(tangent.xyz()), vertex.uv);
}

Raytracer::Raytracer(RaytracerSettings settings, Scene *scene, Image<float, 4> *output)
    : settings(settings),
      scene(scene),
      output(output)
{
    // TODO: make these runtime errors
    assert(scene->getCamera());

    scene->getCamera()->setAspectRatio((float)output->getWidth() / (float)output->getHeight());

    srand((unsigned)time(0));

    addMeshesFromScene();
}

Raytracer::~Raytracer() {
}

void Raytracer::addMeshesFromScene() {
	for (int instanceIdx = 0; instanceIdx < scene->getNumMeshInstances(); instanceIdx++) {
		const MeshInstance *instance = scene->getMeshInstance(instanceIdx);

		float4x4 transform =
	        ::translation(instance->translation.x, instance->translation.y, instance->translation.z) *
	        ::yawPitchRoll(instance->rotation.y, instance->rotation.x, instance->rotation.z) *
	        ::scale(instance->scale.x, instance->scale.y, instance->scale.z);

	    Mesh *mesh = instance->mesh;

	    float4x4 transformInverseTranspose = transpose(inverse(transform));

	    unsigned int materialOffset = materials.size();

	    for (int i = 0; i < mesh->getNumSubmeshes(); i++) {
	        auto submesh = mesh->getSubmesh(i);

	        for (int j = 0; j < submesh->getNumTriangles(); j++) {
	            const Triangle & tri = submesh->getTriangle(j);

	            if (!instance->reverseWinding) {
	                Triangle transformed(
	                    transformVertex(tri.v[0], transform, transformInverseTranspose),
	                    transformVertex(tri.v[1], transform, transformInverseTranspose),
	                    transformVertex(tri.v[2], transform, transformInverseTranspose),
	                    triangles.size(),
	                    submesh->getMaterialID() + materialOffset
	                );

	                triangles.push_back(transformed);
	            }
	            else {
	                Triangle transformed(
	                    transformVertex(tri.v[2], transform, transformInverseTranspose),
	                    transformVertex(tri.v[1], transform, transformInverseTranspose),
	                    transformVertex(tri.v[0], transform, transformInverseTranspose),
	                    triangles.size(),
	                    submesh->getMaterialID() + materialOffset
	                );

	                for (int k = 0; k < 3; k++)
	                    transformed.v[k].normal = -transformed.v[k].normal;

	                triangles.push_back(transformed);
	            }
	        }
	    }

	    for (int i = 0; i < mesh->getNumMaterials(); i++) {
	        const MaterialProperties & props = *mesh->getMaterial(i);

	        auto material = new PBRMaterial(); // TODO: delete

	        material->setDiffuseColor(props.diffuseColor);
	        material->setSpecularColor(props.specularColor);
	        material->setSpecularPower(props.specularPower);
	        material->setReflectivity(props.reflectivity);

	        if (props.diffuseTexture != "") {
	            auto texture = loadTexture(props.diffuseTexture);
	            assert(texture);

	            material->setDiffuseTexture(texture);
	        }

	        if (props.normalTexture != "") {
	            auto texture = loadTexture(props.normalTexture);
	            assert(texture);

	            material->setNormalTexture(texture);
	        }

	        if (props.specularTexture != "") {
	            auto texture = loadTexture(props.specularTexture);
	            assert(texture);

	            material->setRoughnessTexture(texture);
	        }

	        if (props.transparentTexture != "") {
	            auto texture = loadTexture(props.transparentTexture);
	            assert(texture);

	            material->setTransparentTexture(texture);
	        }

	        materials.push_back(material);
	    }
	}
}

void Raytracer::render() {
    shouldShutdown = false;

    KDSAHBuilder builder(tree, triangles, 12.0f, 1.0f);
    builder.build(&_treeStats);

    nBlocksW = (output->getWidth() + BLOCKW - 1) / BLOCKW;
    nBlocksH = (output->getHeight() + BLOCKH - 1) / BLOCKH;
    nBlocks = nBlocksW * nBlocksH;

    currBlockID = 0;

    int nThreads = settings.numThreads;

    if (nThreads == 0)
        nThreads = std::thread::hardware_concurrency(); // TODO: Maybe better way to update image

    numThreadsAlive = nThreads;

	for (int i = 0; i < nThreads; i++)
		workerStats.push_back(RaytracerStats());

	for (int i = 0; i < nThreads; i++) {
		workers.push_back(std::make_shared<std::thread>(std::bind(&Raytracer::worker_thread,
			this, i, nThreads, &workerStats[i])));
	}

    printf("Started %d worker threads\n", nThreads);
}

void Raytracer::shutdown(bool waitUntilFinished, RaytracerStats *stats) {
    if (!waitUntilFinished)
        shouldShutdown = true;

    for (auto& worker : workers)
        worker->join();

	if (stats) {
		memset(stats, 0, sizeof(RaytracerStats));

		int nThreads = workers.size();

		for (int i = 0; i < nThreads; i++) {
			for (int j = 0; j < RaytracerStatCount; j++)
				stats->stat[j] += workerStats[i].stat[j];
		}

		stats->stat[RaytracerStatUnaccountedCycles] = stats->stat[RaytracerStatTotalCycles];

		for (int j = 1; j < RaytracerStatCount - 1; j++)
			stats->stat[RaytracerStatUnaccountedCycles] -= stats->stat[j];
	}

	workerStats.clear();
    workers.clear();
}

void Raytracer::worker_thread(int idx, int numThreads, RaytracerStats *stats) {
	memset(stats, 0, sizeof(RaytracerStats));

	int width = output->getWidth();
    int height = output->getHeight();

    float2 invImageSize = float2(1.0f / (float)width, 1.0f / (float)height);
    float sampleContrib = 1.0f / (float)(settings.pixelSamples * settings.pixelSamples);

    int blockID = idx;

    // TODO: the depth is bounded to 24... no need for such a big stack?
	assert(_treeStats.max_depth < 64);

	// TODO: specialized version which doesn't take max distance
	class RayBuffer {
	private:

		const KDTree & tree;
		util::vector<int2, 16>   pixels[8];
		util::vector<float3, 16> weights[8];
		util::vector<float, 16>  origins[8][3];
		util::vector<float, 16>  directions[8][3];
		util::vector<float, 16>  maxDists[8];
		size_t                   count;
		size_t                   capacity;

	public:

		RayBuffer(const KDTree & tree, size_t capacity)
			: tree(tree),
			  capacity(capacity),
			  count(0)
		{
			for (int i = 0; i < 8; i++) {
				pixels[i].reserve(capacity);
				weights[i].reserve(capacity);
				maxDists[i].reserve(capacity);

				for (int j = 0; j < 3; j++) {
					origins[i][j].reserve(capacity);
					directions[i][j].reserve(capacity);
				}
			}
		}

		void push(const Ray & ray, const int2 & pixel, const float3 & weight, float maxDist) {
			assert(count < capacity);
			
			int c = (signbit(ray.direction.x) << 2) | (signbit(ray.direction.y) << 1) | (signbit(ray.direction.z) << 0);

			pixels[c].push_back_inbounds(pixel);
			weights[c].push_back_inbounds(weight);

			origins[c][0].push_back_inbounds(ray.origin.x);
			origins[c][1].push_back_inbounds(ray.origin.y);
			origins[c][2].push_back_inbounds(ray.origin.z);

			directions[c][0].push_back_inbounds(ray.direction.x);
			directions[c][1].push_back_inbounds(ray.direction.y);
			directions[c][2].push_back_inbounds(ray.direction.z);

			maxDists[c].push_back_inbounds(maxDist);

			count++;
		}

		void flush(
			bool anyCollision,
			std::function<void(const Ray &, const int2 &, const float3 &, float, const Collision &)> hitFunc,
			std::function<void(const Ray &, const int2 &, const float3 &, float)> missFunc)
		{
			// Note: rays now have same sign bits in each direction

			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < (pixels[i].size() & ~(SIMD - 1)); j += SIMD) { // TODO: handle last elements
					PacketCollision<SIMD> result;

					const vector<float, SIMD> (&origin)[3] = {
						*(vector<float, SIMD> *)&origins[i][0][j],
						*(vector<float, SIMD> *)&origins[i][1][j],
						*(vector<float, SIMD> *)&origins[i][2][j]
					};

					const vector<float, SIMD> (&direction)[3] = {
						*(vector<float, SIMD> *)&directions[i][0][j],
						*(vector<float, SIMD> *)&directions[i][1][j],
						*(vector<float, SIMD> *)&directions[i][2][j]
					};

					// Max dist is unused for primary rays
					const vector<float, SIMD> & maxDist = *(vector<float, SIMD> *)&maxDists[i][j]; // TODO: does passing these as args work better?

					vector<bmask, SIMD> hit = tree.intersectPacket(
						origin, direction, maxDist, anyCollision, result);

					StatTimer shadingPack = startStatTimer(RaytracerStatShadingPackCycles);

					// TODO: Might be useful to pass in active mask

					for (int k = 0; k < SIMD; k++) {
						Ray ray;

						ray.origin[0] = origins[i][0][j + k];
						ray.origin[1] = origins[i][1][j + k];
						ray.origin[2] = origins[i][2][j + k];

						ray.direction[0] = directions[i][0][j + k];
						ray.direction[1] = directions[i][1][j + k];
						ray.direction[2] = directions[i][2][j + k];

						float maxDist = maxDists[i][j + k];
						int2 pixel = pixels[i][j + k];
						float3 weight = weights[i][j + k];

						if (hit[k]) {
							Collision collision;

							collision.beta = result.beta[k];
							collision.gamma = result.gamma[k];
							collision.distance = result.distance[k];
							collision.triangle_id = result.triangle_id[k];

							collision.beta = result.beta[k];
							collision.gamma = result.gamma[k];
							collision.distance = result.distance[k];
							collision.triangle_id = result.triangle_id[k];

							hitFunc(ray, pixel, weight, maxDist, collision);
						}
						else {
							missFunc(ray, pixel, weight, maxDist);
						}
					}
				}
			}

			for (int i = 0; i < 8; i++) {
				pixels[i].clear();
				weights[i].clear();

				origins[i][0].clear();
				origins[i][1].clear();
				origins[i][2].clear();

				directions[i][0].clear();
				directions[i][1].clear();
				directions[i][2].clear();

				maxDists[i].clear();
			}

			count = 0;
		}
	};

	int numRays = BLOCKW * BLOCKH * settings.pixelSamples * settings.pixelSamples;

	RayBuffer radianceBuffer(tree, numRays);
	RayBuffer shadowBuffer(tree, numRays);

	struct ShadingWorkItem {
		Ray ray;
		int2 pixel;
		float3 weight;
		float maxDist;
		Collision collision; // TODO: mess with offset of triangle ID
	};

	util::vector<ShadingWorkItem, 16> shadingBuff;
	shadingBuff.reserve(numRays);

	util::vector<ShadingWorkItem, 16> shadowBuff;
	shadowBuff.reserve(numRays);

    while(!shouldShutdown) {
        blockID = currBlockID++;

        if (blockID >= nBlocks)
            break;

		StatTimer totalCycles = startStatTimer(RaytracerStatTotalCycles);

        int y = blockID / nBlocksW;
        int x = blockID % nBlocksW;

        int x0 = BLOCKW * x;
        int y0 = BLOCKH * y;

		for (int y = y0; y < y0 + BLOCKH && y < height; y++) {
			for (int x = x0; x < x0 + BLOCKW && x < width; x++) {
				float3 color(0.0f);

				// TODO: Is the pointer chasing through scene bad?

				// TODO: It's possible to do better sampling

				float2 xy = float2(x, y);

				for (int p = 0; p < settings.pixelSamples; p++) {
					for (int q = 0; q < settings.pixelSamples; q++) {
						StatTimer primaryEmit = startStatTimer(RaytracerStatPrimaryEmitCycles);

						// Take jittered sampled to reduce variance and move from stairstepping
						// artifacts to noise
						float2 xy2 = (xy + randJittered2D(settings.pixelSamples, p, q)) * invImageSize;

						float2 uv = rand2D();
						Ray r = scene->getCamera()->getViewRay(uv, xy2);

						float3 weight(1.0f / (settings.pixelSamples * settings.pixelSamples));

						endStatTimer(stats, primaryEmit);

						StatTimer primaryPack = startStatTimer(RaytracerStatPrimaryPackCycles);
						radianceBuffer.push(r, int2(x, y), weight, INFINITY);
						endStatTimer(stats, primaryPack);
					}
				}
			}
		}

		auto primaryHitFunc = [&](const Ray & ray, const int2 & pixel, const float3 & weight, float maxDist, const Collision & collision)
		{
			ShadingWorkItem item;
			item.ray = ray;
			item.pixel = pixel;
			item.weight = weight;
			item.maxDist = maxDist;
			item.collision = collision;

			assert(shadingBuff.size() < numRays);
			shadingBuff.push_back_inbounds(item);
		};

		auto primaryMissFunc = [&](const Ray & ray, const int2 & pixel, const float3 & weight, float maxDist)
		{
			float3 environmentColor = scene->getEnvironmentColor();
			const Image<float, 4> *environmentMap = scene->getEnvironmentMap();

			if (environmentMap)
				environmentColor = scene->getEnvironmentMapSampler()->sample(environmentMap, ray.direction).xyz();

			float3 color = output->getPixel(pixel.x, pixel.y).xyz();
			color = color + weight * environmentColor; // TODO
			output->setPixel(pixel.x, pixel.y, float4(color, 1.0f));
		};

		// Alternate between tree traversal and shading. Shading may produce more traversal work.
		for (int generation = 0; generation < settings.maxDepth; generation++) {
			radianceBuffer.flush(
				false,
				primaryHitFunc,
				primaryMissFunc);

			StatTimer shadingSort = startStatTimer(RaytracerStatShadingSortCycles);

#if 1
			// TODO: Better sorting is probably possible
			// Ideas: * Insert into buckets or sorted array or hash table
			//        * Bitonic sort
			//        * SIMD sort?
			//        * Shade in SIMD since we're coherent now
			//        * Include material ID in sort
			std::sort(shadingBuff.begin(), shadingBuff.end(), [](const ShadingWorkItem & l, const ShadingWorkItem & r) -> bool {
				return l.collision.triangle_id < r.collision.triangle_id;
			});
#endif

			endStatTimer(stats, shadingSort);

			// TODO: Shorcuts for zero contribution, tiny contribution, no transparency, etc.

			for (auto & item : shadingBuff) {
				StatTimer shading = startStatTimer(RaytracerStatShadingCycles);

				const Triangle *triangle = &triangles[item.collision.triangle_id];

				// TODO: Significant cache miss here pulling vertex data in from memory. Try sorting shading work by
				// triangle?
				Vertex interp = triangle->interpolate(item.collision.beta, item.collision.gamma);
				interp.normal = normalize(interp.normal); // TODO: do we want to do this here?

				const Material *material = materials[triangle->material_id];

				Image<float, 4> *transparentMap = material->getTransparentTexture();
				float opacity = material->getOpacity();

				if (transparentMap) {
					Sampler sampler(Bilinear, Wrap);
					opacity = sampler.sample(transparentMap, interp.uv).w;
				}

				Image<float, 4> *normalMap = material->getNormalTexture();

				if (normalMap) {
					float3 tangent = normalize(interp.tangent);
					float3 bitangent = cross(interp.normal, tangent);

					Sampler sampler(Bilinear, Wrap);
					float3 tbn = sampler.sample(normalMap, interp.uv).xyz() * 2.0f - 1.0f;

					// TODO: extra normalize might not be needed
					interp.normal = normalize(tbn.x * tangent - tbn.y * bitangent + tbn.z * interp.normal);
				}

				float3 wo = -item.ray.direction;

				endStatTimer(stats, shading);

				if (scene->getNumLights() > 0) {
#if 0
					for (int l = 0; l < scene->getNumLights(); l++)
#else
					int l = (int)(rand1D() * scene->getNumLights() * 0.999f);
#endif
					{
						StatTimer shading = startStatTimer(RaytracerStatShadingCycles);

						const Light *light = scene->getLight(l);

						// TODO: Can we jitter in more than one dimensin
						// TODO: importance sampling, multiple importance sampling (need PDF probably)
						float3 lightUV = rand3D();

						float3 wi, Li;
						float r;
						light->sample(lightUV, interp.position, wi, r, Li);

						float ndotl = std::abs(dot(wi, interp.normal)); // TODO: normal mapping

						Ray shadowRay(interp.position + triangle->normal * 0.001f, wi);

						float3 weight = item.weight * Li * material->f(interp, wo, wi) * ndotl * scene->getNumLights() * opacity;

						endStatTimer(stats, shading);

						StatTimer shadowPack = startStatTimer(RaytracerStatShadowPackCycles);

						shadowBuffer.push(shadowRay, item.pixel, weight, r * 0.999f);

						// TODO: If light does not cast shadows, return color immediately
						endStatTimer(stats, shadowPack);
					}
				}

				float pdf = 1.0f;
				float p_kill = 0.95f;
				bool kill = false;

				if (generation == settings.maxDepth - 1)
					kill = true;
				else if (generation >= 1 && rand1D() <= p_kill) {
					pdf *= p_kill;
					kill = true;
				}
				
				if (!kill) {
					Ray indirectRay;
					float3 indirectWeight;

					float p_transparent = 0.0f;
					float p_indirect = 1.0f;

					if (opacity < 1.0f)
						p_transparent = 1.0f;
					// TOD: if reflection...

					float p_sum = p_transparent + p_indirect;

					if (p_sum > 0.0f) {
						StatTimer secondaryEmit = startStatTimer(RaytracerStatSecondaryEmitCycles);

						p_transparent /= p_sum;
						p_indirect /= p_sum;

						float x = rand1D();

						if (x <= p_transparent) {
							pdf *= p_transparent;

							indirectRay.origin = interp.position + item.ray.direction * 0.01f;
							indirectRay.direction = item.ray.direction;

							// Importance sampling: n dot l term cancels out
							indirectWeight = item.weight * (1.0f - opacity) / pdf;
						}
						else if (x - p_transparent <= p_indirect) {
							pdf *= p_indirect;

							float2 rand = rand2D();

							indirectRay.origin = interp.position + triangle->normal * 0.001f;
							indirectRay.direction = mapCosHemisphere(1.0f, rand);
							indirectRay.direction = alignHemisphere(indirectRay.direction, triangle->normal);

							// Importance sampling: n dot l term cancels out
							indirectWeight = item.weight * material->f(interp, wo, indirectRay.direction) * (float)M_PI / pdf;
						}
						else if (false) {
							indirectRay.origin = interp.position + triangle->normal * 0.001f;
							indirectRay.direction = reflect(wo, interp.normal);
							indirectWeight = item.weight * material->getReflectivity(); // TODO: hack
							//float ndotl = saturate(dot(r.direction, interp.normal));
							//weight *= material->f(interp, wo, r.direction) * ndotl;
						}

						endStatTimer(stats, secondaryEmit);

						StatTimer secondaryPack = startStatTimer(RaytracerStatSecondaryPackCycles);

						radianceBuffer.push(indirectRay, item.pixel, indirectWeight, INFINITY);

						endStatTimer(stats, secondaryPack);
					}
				}
			}

			shadingBuff.clear();

			for (int k = 0; k < 3; k++) {
				shadowBuffer.flush(
					true, 
					[&](const Ray & ray, const int2 & pixel, const float3 & weight, float maxDist, const Collision & collision) {
						// TODO: Terminate eventually

						if (k < 2) {
							ShadingWorkItem item;

							item.ray = ray;
							item.pixel = pixel;
							item.weight = weight;
							item.maxDist = maxDist;
							item.collision = collision;

							assert(shadowBuff.size() < numRays);
							shadowBuff.push_back_inbounds(item);
						}
					},
					[&](const Ray & ray, const int2 & pixel, const float3 & weight, float maxDist) {
						float3 color = output->getPixel(pixel.x, pixel.y).xyz();
						color = color + weight; // TODO
						output->setPixel(pixel.x, pixel.y, float4(color, 1.0f));
					});

				for (auto & item : shadowBuff) {
					// TODO: Only do this for transparent objects/triangles
					const Triangle *triangle = &triangles[item.collision.triangle_id];

					// TODO: Significant cache miss here pulling vertex data in from memory. Try sorting shading work by
					// triangle?
					// TODO: We only need the UV. Also, in general, position can be created from O + tD, which is probably
					// cheaper than interpolating.

					float alpha = 1.0f - item.collision.beta - item.collision.gamma;
					float3 position = item.ray.origin + item.collision.distance * item.ray.direction;
					float2 uv = triangle->v[0].uv * alpha + triangle->v[1].uv * item.collision.beta + triangle->v[2].uv * item.collision.gamma;

					//Vertex interp = triangle->interpolate(item.collision.beta, item.collision.gamma);
					//interp.normal = normalize(interp.normal); // TODO: do we want to do this here?

					const Material *material = materials[triangle->material_id];

					Image<float, 4> *transparentMap = material->getTransparentTexture();
					float opacity = material->getOpacity();

					if (transparentMap) {
						Sampler sampler(Bilinear, Wrap);
						opacity = sampler.sample(transparentMap, uv).w;
					}

					if (opacity < 1.0f) {
						Ray shadowRay;
						
						shadowRay.origin = position + item.ray.direction * 0.01f;
						shadowRay.direction = item.ray.direction;

						// Importance sampling: n dot l term cancels out
						float3 shadowWeight = item.weight * (1.0f - opacity);

						shadowBuffer.push(shadowRay, item.pixel, shadowWeight, item.maxDist - item.collision.distance);
					}
				}

				shadowBuff.clear();
			}
		}

        // TODO: Flushing one tile at a time keeps the tile in the cache probably, but might
        // not get the most coherence. Adjusting the tile size would affect this probably.

		endStatTimer(stats, totalCycles);
    }

    //std::cout << "Ray buffer size: " << rayBuff.capacity() << " (" << (rayBuff.capacity() * sizeof(Ray) + 1024 - 1) / 1024 << "kb)" << std::endl;

    numThreadsAlive--;
}

bool Raytracer::intersect(float2 uv, Collision & result) {
	Ray r = scene->getCamera()->getViewRay(float2(0, 0), uv);

	return tree.intersect(r, INFINITY, result);
}
