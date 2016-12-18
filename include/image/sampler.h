/**
 * @file sampler.h
 *
 * @brief Image sampler
 *
 * @author Sean James <seanjames777@gmail.com>
 */

#ifndef __SAMPLER_H
#define __SAMPLER_H

#include <image/image.h>

// TODO: Mipmapping and min/mag filter.
// TODO: Might be faster to use template filter settings for inlining

/**
 * @brief Options for how to filter multiple image samples
 */
enum FilterMode {
    Nearest,   //!< Take the nearest sample
    Bilinear,  //!< Combine four nearest pixels
};

/**
 * @brief Options for how to treat pixel coordinates outside an image when
 * sampling
 */
enum BorderMode {
    Clamp,  //!< Clamp pixel address to edge of image
    Wrap,   //!< Wrap pixel address around border of image
	Mirror  //!< Wrap mirrored
};

/**
 * @brief Image sampler. Takes one or more samples from an image and combines
 * them to produce a pixel value.
 */
class RT_EXPORT Sampler {
private:
    
    FilterMode filter; //!< Sample filtering settings
    BorderMode border; //!< Border mode setting

    /**
     * @brief Get a pixel value handling out-of-bounds pixel coordinates
     *
     * @param[in] image Image to sample from
     * @param[in] x     Pixel X coordinate
     * @param[in] y     Pixel Y coordinate
     *
     * @return Pixel value
     */
    template<typename T, unsigned int C>
    inline vector<T, C> sampleBorder(const Image<T, C> *image, int x, int y) const {
        // TODO: Note: power of two only

        int width = image->getWidth();
        int height = image->getHeight();

        // TODO: Can replace the branch with template or just a generic and+or sequence
        switch(border) {
        case Clamp:
            x &= ~(x >> 31);            // Less than 0, fill 0's
            x |= (width - 1 - x) >> 31; // Greater than width - 1, fill 1's
            y &= ~(y >> 31);
            y |= (height - 1 - y) >> 31;
        case Wrap:
            x &= (width - 1);           // Wrap around width - 1
            y &= (height - 1);
            break;
		case Mirror:
			// TODO
			x = (x & width) ? (width - (x & (width - 1)) - 1) : (x & (width - 1));
			y = (y & height) ? (height - (y & (height - 1)) - 1) : (y & (height - 1));
			break;
        }

        return image->getPixel(x, y);
    }

public:

    /**
     * @brief Constructor
     *
     * @param[in] filter Sample filtering mode
     * @param[in] border Border address mode
     */
    Sampler(FilterMode filter, BorderMode border)
        : filter(filter),
          border(border)
    {
    }

    /**
     * @brief Destructor
     */
    ~Sampler() {
    }

    /**
     * @brief Sample from an image
     *
     * @param[in] image Image to sample from
     * @param[in] uv    UV coordinates of sample. (0, 0) represents top-left of image
     *                  and (1, 1) represents bottom-right.
     *
     * @return Color value
     */
    template<typename T, unsigned int C>
    inline vector<T, C> sample(const Image<T, C> *image, const float2 & uv) const
    {
        // TODO: Might be worth doing a fancier filter

        float x = uv.x * (image->getWidth() - 1);
        float y = uv.y * (image->getHeight() - 1);

        int x0 = (int)x;
        int y0 = (int)y;

        switch(filter) {
        default:
        case Nearest:
            return sampleBorder(image, x0, y0);
        case Bilinear:
            // These should be nearby in the cache due to tiling
            vector<T, C> s0 = sampleBorder(image, x0,     y0);
			vector<T, C> s1 = sampleBorder(image, x0 + 1, y0);
			vector<T, C> s2 = sampleBorder(image, x0,     y0 + 1);
			vector<T, C> s3 = sampleBorder(image, x0 + 1, y0 + 1);

            float du = x - x0;
            float dv = y - y0;

			vector<T, C> t0 = du * s1 + (1.0f - du) * s0;
			vector<T, C> t1 = du * s3 + (1.0f - du) * s2;

            return dv * t1 + (1.0f - dv) * t0;
        }
    }

    /**
     * @brief Sample from an environment map
     *
     * @param[in] image Environment map to sample from
     * @param[in] norm  Normalized direction to sample from
     *
     * @return Color value
     */
    template<typename T, unsigned int C>
    inline vector<T, C> sample(const Image<T, C> *image, const float3 & norm) const
    {
        // TODO: This atan2 and acosf is super expensive
        float2 uv = (float2){ atan2f(norm.z, norm.x) + (float)M_PI, acosf(norm.y) };
        uv = uv / (float2){ 2.0f * (float)M_PI, (float)M_PI };

        return sample(image, uv);
    }

};

#endif
