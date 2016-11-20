# Radiometry Notes

Partially summarized from Physically Based Rendering: From Theory to Implementation.

## Units

### Flux

The total amount of energy passing through a region of space per unit time, measured in joules/second = watts. Total emission from a light source generally measured in Flux through a sphere surrounding the light source (radius independent). Denoted by $\Phi$.

### Irradiance and Radiant Exitance

Flux per unit area arriving at a surface (irradiance) or leaving a surface (radiant exitance), measured in watts/m^2. Irradiance denoted by $E$. Can be defined as $E$ = $\frac{d\Phi}{dA}$. For a sphere surrounding a light source, $E = \frac{\Phi}{4 \pi r^2}$, so energy recieved falls off with the squared distance to the light. If a surface is at an angle to the light source, projected area is proportional to $\cos{\theta}$, so $E = \frac{\Phi}{A} \cos{\theta}$.

### Solid Angle

Area of an object projected onto a unit sphere, measured in steradians. Entire sphere subtends a solid angle of $4\pi$, hemisphere subtends $2\pi$. Denoted by $\omega$.

### Intensity

Flux per solid angle, denoted by $I$, where $I = \frac{d\Phi}{d\omega}$.

### Radiance

Flux per unit area per unit solid angle, denoted by $L$, where $L = \frac{d\Phi}{d\omega\ dA_\perp}$, where $A_\perp$ is the projected area of $dA$ on a surface perpendicular to $\omega$. Measurement of incoming light over a small area on a surface from a small cone of incoming directions. Remains constant along $\omega$.

Radiance can either arrive at or leave a surface: incident or exitant radiance, denoted $L_i$ or $L_o$, usually as a function of position $p$ and $\omega$. In general $L_i(p, \omega) \neq L_o(p, \omega)$, but $L_i(p, \omega) = L_o(p, -\omega)$ if there is no surface or participating media.

Other quantities can be derived from radiance by integration. For example, irradiance is integral of radiance over hemisphere above surface.

$$
\begin{align}
            E &= \frac{\Phi}{A} \cos{\theta} \\
\frac{dE}{d\omega} &= \frac{d\Phi}{d\omega\ dA} \cos{\theta} = L_i \cos{\theta} \\
            E &= \int_\Omega {L_i \cos{\theta}\ dw} \\
\end{align}
$$

## Integration

### Integrals Over Spherical Coordinates

$\theta$ is elevation, $\Phi$ is rotation around Z (vertical) axis.

$$
\begin{align}
x &= \sin{\theta} \cos{\Phi} \\
y &= \sin{\theta} \sin{\Phi} \\
z &= \cos{\theta}
\end{align}
$$

Inversely:

$$\theta = \arccos{z}$$

$$
\begin{align}
\arctan{\left(\frac{y}{x}\right)} &= \arctan{\left(\frac{\sin{\theta} \sin{\Phi}}{\sin{\theta} \cos{\Phi}}\right)} \\
                                  &= \arctan{\left(\frac{\sin{\Phi}}{\cos{\Phi}}\right)} \\
                                  &= \arctan{\left(\tan{\Phi}\right)} \\
                                  &= \Phi
\end{align}
$$

An integral over a hemisphere is an integral over a small patch with differential edge dimensions $\sin{\theta}\ d\theta$ and $d\phi$, so:

$$d\omega = \sin{\theta}\ d\theta\ d\phi$$

For example, irradiance $E$ is the integral of radiance $L_i$ over a hemisphere $\Omega$:

$$
\begin{align}
E(p, n) &= \int_{\Omega} L_i(p, \omega_i) \cos{\theta}\ d\omega_i \\
        &= \int_{\phi=0}^{2\pi}{
              \int_{\theta=0}^{\frac{\pi}{2}}{
                  L_i(p, \theta, \phi) \cos{\theta} \sin{\theta}\ d\theta\ d\phi
              }
          }
\end{align}
$$

If incoming radiance is constant:

$$
E(p, n) = L_i \int_{\phi=0}^{2\pi}{
              \int_{\theta=0}^{\frac{\pi}{2}}{
                  \cos{\theta} \sin{\theta}\ d\theta\ d\phi
              }
          }
$$

Let $u(\theta) = \cos{\theta}$. Then $du = -\sin{\theta}$. So:

$$
\begin{align}
E(p, n) &= -L_i \int_{\phi=0}^{2\pi}{
                \int_{\theta=0}^{\frac{\pi}{2}}{
                    u\ du\ d\phi
                }
            } \\
        &= -L_i \int_{\phi=0}^{2\pi}{
                \frac{1}{2} u^2 \Big|_{u=1}^{u=0}\ d\phi
            } \\
        &= \frac{1}{2} L_i \int_{\theta=0}^{2\pi}{
                d\phi
            } \\
        &= \pi L_i
\end{align}
$$

### Integrals Over Area

Consider incoming radiance from a quad at point $p$. Can integrate over surface of quad to compute irradiance. Let $p'$ be a point on the quad, $\theta_o$ be the angle between the surface normal at $p'$ and the direction from $p'$ to $p$, and $\theta_i$ be the angle between the surface normal at point $p$ and the direction from $p$ to $p'$. Then:

$$
\begin{align}
d\omega &= \frac{dA\ \cos{\theta_o}}{r^2} \\
E(p, n) &= \int_A L \cos{\theta_i} \frac{\cos{\theta_o}\ dA}{r^2}
\end{align}
$$

## Surface Reflection

### Bidirectional Reflectance Distribution Function

The Bidirectional Reflectance Distribution Function (BRDF), denoted $f_r$, describes the proportion of radiance $L_o(p, \omega_o)$ reflected towards the viewer along $\omega_o$ from radiance $L_i(p, \omega_i)$ arriving from a light source along $\omega_i$.

$$dL_o(p, \omega_i) = f_r(p, \omega_o, \omega_i) L_i(p, \omega_i) \cos{\theta_i}\ d\omega_i
                    = f_r(p, \omega_o, \omega_i)\ dE(p, \omega_i) $$

Physically based BRDFs have two important properties:

* Reciprocity: $$f_r(p, \omega_o, \omega_i) = f_r(p, \omega_i, \omega_o)$$
* Conservation of Energy: $$\int_\Omega f_r(p, \omega_o, \omega_i) \cos{\theta_i}\ d\omega_i \leq 1$$

By integrating over a hemisphere, we obtain the rendering equation, which computes the outgoing radiance due to incoming radiance from all directions:

$$L_o(p, \omega_o) = \int_\Omega f_r(p, \omega_o, \omega_i) L_i(p, \omega_i) \cos{\theta_i}\ d\omega_i$$

Similar to the BRDF, the Bidirectional Transmittance Distribution Function (BTDF), denoted $f_t$ describes the amount of light transmitted all the way through a surface. The Bidirectional Scattering Distribution Function (BSDF) combines the BRDF and BTDF.

Finally, the Bidirectional Scattering-Surface Distribution Function (BSSRDF) describes the amount of light scattered from one point and incoming direction to another point and outgoing direction through an object.

### Lambertian Reflection

The Lambertian reflection model reflects a constant fraction $D$ of incoming light equally in all directions. This ratio is divided by $\pi$ to maintain conservation of energy.

$$f_r(p, \omega_o, \omega_i) = \frac{D}{\pi}$$

### Phong Specular Reflection

The Phong specular reflection model reflects the most light along the reflection $r$ of $\omega_i$ across the surface normal $n$. The "sharpness" of the highlight is controlled with a parameter $\alpha$.

$$f_r(p, \omega_o, \omega_i) = \left(r \cdot \omega_o\right)^\alpha$$

For this model to conserve energy, for all $\omega_o$:

$$\int_\Omega \left(r \cdot \omega_o\right)^\alpha \left(n \cdot \omega_i\right)\ d\omega_i \leq 1$$

The integrand is maximized when $r = \omega_o$, or $r = n$. Because $r$ is the reflection of $\omega_i$ across the normal $n$, $n \cdot \omega_o = n \cdot \omega_i$. So, we can rewrite the integral as:

$$
\int_{\phi=0}^{2\pi}{
    \int_{\theta=0}^{\frac{\pi}{2}}{
        \cos^{\alpha+1}{\theta} \sin{\theta}\ d\theta\ d\phi
    }
}
$$

Let $g(x) = \cos{x}$. Then $g'(x) = -\sin{x}$. Let $f(x) = -x^{\alpha + 1}$. We can again rewrite the integral as:

$$
\begin{align}
&\int_{\phi=0}^{2\pi}{
    \int_{\theta=0}^{\frac{\pi}{2}}{
        f(g(\theta)) g'(\theta)\ d\theta\ d\phi
    }
} \\
=&\int_{\phi=0}^{2\pi}{
    \int_{u=1}^{0}{
        f(u)\ du\ d\phi
    }
} \\
=&\int_{\phi=0}^{2\pi}{
    \int_{u=1}^{0}{
        -u^{\alpha + 1}\ du\ d\phi
    }
} \\
=&\int_{\phi=0}^{2\pi}{
        -\frac{1}{\alpha + 2} u^{\alpha + 2}\ \big|_{u=1}^{0}\ d\phi
} \\
=&\frac{1}{\alpha + 2} \int_{\phi=0}^{2\pi}{
        d\phi
} \\
=&\frac{2 \pi}{\alpha + 2}
\end{align}
$$

So, for this model to conserve energy:

$$f_r(p, \omega_o, \omega_i) = \frac{\alpha+2}{2 \pi}(r \cdot \omega_o)^\alpha$$

### Blinn-Phong Specular Reflection

Blinn-Phong is similar to Phong, except that the half-angle vector $h$ is used instead of the reflection vector, where $h = \frac{\omega_i + \omega_o}{||\omega_i + \omega_o||}$. Then:

$$f_r(p, \omega_o, \omega_i) = (n \cdot h)^\alpha$$

The normalization constant for Blinn-Phong is between $\frac{\alpha + 6}{8 \pi}$ and $\frac{\alpha + 8}{8 \pi}$, so the latter is often chosen as a consevative approximation [3]:

$$f_r(p, \omega_o, \omega_i) = \frac{\alpha + 8}{8 \pi} (n \cdot h)^\alpha$$

### Cook-Torrance Specular Reflection

Cook-Torrance is a microfacet reflection model, which models the surface as a series of microsopic reflectors. These microfacets only reflect light towards the viewer when the half angle vector $h$, given by $h = \frac{\omega_i + \omega_o}{||\omega_i + \omega_o||}$, is aligned with the microfacet's surface normal.

Some of the incoming radiance is blocked by other microfacets before it can be reflected back towards the viewer (shadowing), and some of the outgoing radiance is blocked by other microfacets before it can reach the viewer (masking).

The BRDF is given by:

$$f_r(p, \omega_o, \omega_i) = \frac{F(\omega_i, h) D(h) G(\omega_i, \omega_o, h)}{4(n \cdot \omega_i)(n \cdot \omega_o)}$$

$F$ is the Fresnel reflectance function, which determines the ratio of light reflected from active microfacets, whose surface normal $m=h$. $D$ is the normal distribution function which determines the ratio of active microfacets. Finally, $G$ is the geometry function, which determines the ratio of active microfacets which are not shadowed or masked. $F$, $G$, and $H$ can be chosen independently from a variety of functions.

> TODO: Explain the $4(n \cdot \omega_i)(n \cdot \omega_o)$ terms

#### Fresnel

The Schlick approximation to the full Fresnel equation is commonly used. In general, $\theta$ is the angle between the surface normal and the view (or reflection) direction. However, for microfacets the  angle between the view or reflection direction and the microfacet normal $h$ is used.

$$
F(\omega_i, h) = F_0 + (1 - F_0)(1 - \cos{\theta})^5 \\
F_0 = \left(\frac{\eta_1 - \eta_2}{\eta_1 + \eta_2}\right)^2
$$

$\eta_1$ and $\eta_2$ are the indices of refraction of the material and medium. $F_0$ is usually provided as a material parameter assuming one of $\eta_1$ or $\eta_2$ is the index of refraction of air.

> TODO: Conductors

#### Beckmann Distribution Function

One choice of distribution function is the Beckmann distribution function, where $m_B$ is the surface roughness.

$$D_{Beckmann}(h) = \frac{e^{\frac{(n \cdot h)^2-1}{m^2(n \cdot h)^2}}}{\pi m^2 (n \cdot h)^4}$$

Note that $(n \cdot h)$ should be clamped to zero and a small epsilon should be added to avoid division by zero.

#### Blinn-Phong Distribution Function

Another choice of distribution function is the Blinn-Phong distribution function.

$$D_{Blinn-Phong}(h) = \frac{\alpha+2}{2\pi} (n \cdot h)^\alpha$$

Note that $m = \sqrt{\frac{2}{\alpha+2}}$ in the Beckmann distribution.

#### GGX Distribution Function

The GGX distribution function is given by:

$$D_{GGX}(h) = \frac{m^2}{\pi ((n \cdot h)^2 (m^2 - 1) + 1)^2}$$

#### Implicit Geometry Function

There are many choices of geometry function. The simplest is the "implicit" geometry function, where:

$$G_{Implicit}(\omega_i, \omega_o, h) = (n \cdot \omega_i)(n \cdot \omega_o)$$

This has the effect of cancelling out most of the denominator in the full BRDF:

$$f_r(p, \omega_o, \omega_i) = \frac{F(\omega_i, h) D(h)}{4}$$

This is easy to compute, but will result in overly dark specular highlights with larger roughness values.

#### Cook-Torrance Geometry Function

The Cook-Torrance geometry function is given by:

$$G_{Cook-Torrance}(\omega_i, \omega_o, h)=\min\left(1, 
    \frac{2(n \cdot h)(n \cdot v)}{(v \cdot h)},
    \frac{2(n \cdot h)(n \cdot l)}{(v \cdot h)}
\right)$$

#### Smith Geometry Function

The Smith geometry function is broken into two more functions $G'(v, h)$:

$$G_{Smith}(\omega_i, \omega_o, h)=G_1(l,h)\ G_1(v,h)$$

There are numerous choices for $G_1(v, h)$ as well.

##### Beckmann $G_1$ Function

The Beckmann $G_1$ function is given by:

$$
G_{Beckmann}(v, h) =
\begin{cases}
\frac{3.535c + 2.181c^2}{1 + 2.276c + 2.557c^2} & c \lt 1.6 \\
1                                               & c \geq 1.6
\end{cases} \\
c = \frac{n \cdot v}{m \sqrt{1- (n \cdot v)^2)}}
$$

##### Schlick's $G_1$ Function

Schlick's approximation to $G_{Beckmann}(v,h)$ is given by:

$$G_{Schlick}(v,h) = \frac{(n \cdot v)}{(n \cdot v)(1-k)+k}$$

Where $k=m \sqrt{\frac{2}{\pi}}$.

> * TODO: Many of these terms need to be clamped to 0, but some do not

##### GGX $G_1$ Function

The GGX $G_1$ function is given by:

$$G_{GGX}(v, h) = \frac{2 (n \cdot v)}{(n \cdot v) + \sqrt{m^2 + (1-m^2)(n \cdot v)^2}}$$

### Combined Diffuse and Specular

To conserve energy when combining a diffuse and specular BRDF, $k_d$ and $k_s$ must be chosen such that, for all $\omega_o$:

$$\int_\Omega (k_d\ f_d(p, \omega_o, \omega_i) + k_s\ f_s(p, \omega_o, \omega_i)) \left(n \cdot \omega_i\right)\ d\omega_i \leq 1$$

Assuming both $f_d$ and $f_s$ are energy conserving, this simplifies to:

$$k_d + k_s \leq 1$$

The ratio between $k_s$ and $k_d$ is often driven by a "metalness" parameter of the material.

> TODO: Other ways to conserve energy using $F(\theta)$ or $F_0$.

### Punctual Light Sources

Classical point, spot, and directional light sources are often parameterized using a value $c_{light}$ which represents the radiance reflected from a fully reflective pure Lambertian surface, assuming the surface normal is parallel to the light direction $l_c$. For such a surface $f_r(p, \omega_o, \omega_i) = \frac{1}{\pi}$, so:

$$
c_{light} = \frac{1}{\pi} \int_\Omega L_i(p, \omega_i) \cos{\theta_i}\ d\omega_i
          = \frac{1}{\pi} L_i
$$

Therefore, $L_i = \pi c_{light}$. To apply these light sources to a general BRDF:

$$
L_o(p, \omega_o) = \int_\Omega f_r(p, \omega_o, \omega_i) L_i(p, \omega_i) \cos{\theta_i}\ d\omega_i
                 = \pi\ f_r\left(p, \omega_o, \omega_i\right) c_{light} \cos{\theta_i}
$$

In the case of a Lambertian BRDF, this simplifies to:

$$L_o(p, \omega_o) = D\ c_{light} \cos{\theta_i}$$

### Environment Maps

Assuming a Lambertian surface:

$$L_o(p, \omega_o) = \frac{D}{\pi} \int_\Omega L_i(p, \omega_i) \cos{\theta_i}\ d\omega_i$$

This integral can be precomputed for all values of $n$.

### References

1. Physically Based Rendering: From Theory to Implementation
2. [Physically Based Shading Models in Film and Game Production](http://renderwonk.com/publications/s2010-shading-course/hoffman/s2010_physically_based_shading_hoffman_a_notes.pdf)
3. [The Blinn-Phong Normalization Zoo](http://www.thetenthplanet.de/archives/255)
4. [A Reflectance Model for Computer Graphics](http://inst.cs.berkeley.edu/~cs294-13/fa09/lectures/cookpaper.pdf)
5. Physically Based Lighting in Black Ops
