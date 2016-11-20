// TODO: extend with BTDF
class BRDF {
public:

    // Evaluate the BRDF for incoming light direction wi and outgoing light direction wo
    virtual float3 f(float3 wo, float3 wi) const = 0;

#if 0
    // For delta distributions such as perfect specular reflection, calculate the incoming
    // light direction and evaluate the BRDF for outgoing light direction wo
    virtual float3 sample_f(float3 wo, float3 & wi, float u1, float u2, float & wi_pdf) const
    {
        // TODO: cosine sample hemisphere. for mirror reflection, delta distribution.
        wi = ...;

        wi_pdf = pdf(wo, wi);

        return f(wo, wi);
    };

    virtual float pdf(float3 wo, float3 wi) const {
        // TODO: assumes normal is up
        return abs(wi.z) / (float)M_PI;
    }
#endif
};
