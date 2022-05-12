#include <string>
#include <iostream>
#include <vector>
#include <tuple>
#include <ranges>
#include "image_ppm.h"

#define applyToRgb(func,orig,res) func(std::ranges::transform_view(orig,[](auto x){return x.r;}),begin(std::ranges::transform_view(res,[](auto& x)->auto&{return x.r;})));\
func(std::ranges::transform_view(orig,[](auto x){return x.g;}),begin(std::ranges::transform_view(res,[](auto& x)->auto&{return x.g;})));\
func(std::ranges::transform_view(orig,[](auto x){return x.b;}),begin(std::ranges::transform_view(res,[](auto& x)->auto&{return x.b;})));

using namespace std::string_literals;

OCTET round_to_byte(double val)
{
    return (OCTET)std::round(val);
}

OCTET round_clamped(double val)
{
    return (OCTET)std::round(std::clamp(val,0.,255.));
}

OCTET round_offset(double val)
{
    return (OCTET)std::round(val+128);
}

OCTET round_offset_clamped(double val)
{
    return (OCTET)std::clamp(std::round(val+128), 0., 255.);
}

double unoffset(OCTET val)
{
    return val-128.;
}

struct Rgb;

struct YCbCr
{
    OCTET y,cb,cr;
    operator Rgb() const;
    auto operator<=>(const YCbCr&) const = default;
};

struct Rgb
{
    OCTET r,g,b;
    operator YCbCr() const
    {
        return { round_to_byte(std::clamp(0.299*r + 0.587*g + 0.114*b, 0., 255.)),
            round_to_byte(std::clamp(-0.1687*r -0.3313*g + 0.5*b + 128, 0., 255.)),
            round_to_byte(std::clamp(0.5*r - 0.4187*g - 0.0813*b + 128, 0., 255.))
        };
    }
    auto operator<=>(const Rgb&) const = default;
};

YCbCr::operator Rgb() const
{
    return { round_to_byte(std::clamp(y + 1.402*(cr-128), 0., 255.)),
        round_to_byte(std::clamp(y - 0.34414*(cb-128) - 0.71414*(cr-128), 0., 255.)),
        round_to_byte(std::clamp(y + 1.772*(cb-128), 0., 255.))
    };
}

constexpr double sqr(double x) { return x*x; }

constexpr double sqrDist(const Rgb& l, const Rgb& r)
{
    return sqr((double)l.r - (double)r.r) + sqr((double)l.g - (double)r.g) + sqr((double)l.b - (double)r.b);
}

template<typename T>
std::tuple<std::vector<T>, int, int> charger(std::string_view nom_image) requires std::same_as<T,Rgb> || std::same_as<T,OCTET>
{
    constexpr bool use_rgb = std::is_same_v<T, Rgb>;
    int h, w;
    if constexpr (use_rgb) lire_nb_lignes_colonnes_image_ppm(nom_image.data(), &h, &w);
    else lire_nb_lignes_colonnes_image_pgm(nom_image.data(), &h, &w);
    
    size_t n_pixels = h*w;
    size_t data_size = use_rgb ? n_pixels * 3 : n_pixels;
    
    std::vector<OCTET> img(data_size);
    if constexpr(!use_rgb)
    {
        lire_image_pgm(nom_image.data(), img.data(), n_pixels);
        return {img, h, w};
    }
    else
    {
        lire_image_ppm(nom_image.data(), img.data(), n_pixels);

        std::vector<Rgb> result(n_pixels);
        size_t i=0;
        for(Rgb& rgb : result)
        {
            rgb.r = img[i++];
            rgb.g = img[i++];
            rgb.b = img[i++];
        }

        return {result, h, w};
    }
}

void sauvegarder(std::string_view nom_image, const std::vector<Rgb>& img, int nb_lignes, int nb_colonnes)
{
    std::vector<OCTET> img_raw(img.size() * 3);
    size_t i = 0;
    for(Rgb pixel : img)
    {
        img_raw[i++] = pixel.r;
        img_raw[i++] = pixel.g;
        img_raw[i++] = pixel.b;
    }
    ecrire_image_ppm(nom_image.data(), img_raw.data(), nb_lignes, nb_colonnes);
}

void sauvegarder(std::string_view nom_image, const std::vector<OCTET>& img, int nb_lignes, int nb_colonnes)
{
    ecrire_image_pgm(nom_image.data(), img.data(), nb_lignes, nb_colonnes);
}

double psnr(const std::vector<Rgb>& l, const std::vector<Rgb>& r)
{
    size_t n = l.size();
    double error = 0;

    for(size_t i = 0; i < n; ++i)
    {
        error += sqrDist(l[i], r[i]);
    }

    error /= n;

    return 10*std::log10(sqr(255)/error);
}

template<size_t N>
constexpr std::array<std::array<bool,N>,N> makeDefaultNeighbors()
{
    const size_t center = N/2;
    std::array<std::array<bool,N>,N> result;
    for(size_t i = 0; i < N; ++i)
    {
        for(size_t j = 0; j < N; ++j)
        {
            result[i][j] = sqr((double)i-center) + sqr((double)j-center) <= sqr(center);
        }
    }

    return result;
}

template<size_t N>
std::vector<OCTET> dilater_couleur(const std::vector<OCTET>& img, int h, int w, OCTET col, const std::array<std::array<bool,N>,N>& neighbors)
{
    const size_t neighbor_center = N/2;
    std::vector<OCTET> result = img;
    for(size_t i = 0; i < h; ++i)
    {
        for(size_t j = 0; j < w; ++j)
        {
            if(img[i*h+j] != col) continue;
            for(size_t k = 0; k < N; ++k)
            {
                if(k < neighbor_center && neighbor_center - k > i) continue;
                if(k > neighbor_center && h - (k - neighbor_center) <= i) continue;

                const ptrdiff_t k_offset = k - neighbor_center;
                for(size_t l = 0; l < N; ++l)
                {
                    if(!neighbors[k][l]) continue;
                    if(l < neighbor_center && neighbor_center - l > j) continue;
                    if(l > neighbor_center && w - (l - neighbor_center) <= j) continue;
                    const ptrdiff_t l_offset = l - neighbor_center;

                    result[(i+k_offset)*h+j+l_offset] = col;
                }
            }
        }
    }

    return result;
}

template<size_t N, typename F = std::identity>
std::vector<OCTET> filter(const std::vector<OCTET>& img, int h, int w, const std::array<std::array<double,N>,N>& f, F&& transformation = {})
{
    const size_t neighbor_center = N/2;
    std::vector<OCTET> result = img;
    for(size_t i = 0; i < h; ++i)
    {
        for(size_t j = 0; j < w; ++j)
        {
            double sum = 0;
            for(size_t k = 0; k < N; ++k)
            {
                const ptrdiff_t k_offset = k < neighbor_center && neighbor_center - k > i || k > neighbor_center && h - (k - neighbor_center) <= i?
                    0 : k - neighbor_center;

                for(size_t l = 0; l < N; ++l)
                {
                    const ptrdiff_t l_offset = l < neighbor_center && neighbor_center - l > j || l > neighbor_center && w - (l - neighbor_center) <= j?
                        0 : l - neighbor_center;
                    sum += img[(i+k_offset)*h+j+l_offset] * f[k][l];
                }
            }
            result[i*w+j] = transformation(sum);
        }
    }

    return result;
}

template<size_t N>
std::vector<Rgb> blur(const std::vector<Rgb>& img, int h, int w, const std::array<std::array<bool,N>,N>& neighbors)
{
    const size_t neighbor_center = N/2;
    std::vector<Rgb> result = img;
    for(size_t i = 0; i < h; ++i)
    {
        for(size_t j = 0; j < w; ++j)
        {
            double sum_r = 0;
            double sum_g = 0;
            double sum_b = 0;
            unsigned count = 0;
            for(size_t k = 0; k < N; ++k)
            {
                if(k < neighbor_center && neighbor_center - k > i) continue;
                if(k > neighbor_center && h - (k - neighbor_center) <= i) continue;

                const ptrdiff_t k_offset = k - neighbor_center;
                for(size_t l = 0; l < N; ++l)
                {
                    if(!neighbors[k][l]) continue;
                    if(l < neighbor_center && neighbor_center - l > j) continue;
                    if(l > neighbor_center && w - (l - neighbor_center) <= j) continue;
                    const ptrdiff_t l_offset = l - neighbor_center;

                    sum_r += img[(i+k_offset)*h+j+l_offset].r;
                    sum_g += img[(i+k_offset)*h+j+l_offset].g;
                    sum_b += img[(i+k_offset)*h+j+l_offset].b;
                    ++count;
                }
            }

            if(count)
            {
                result[i*h+j].r = sum_r/count;
                result[i*h+j].g = sum_g/count;
                result[i*h+j].b = sum_b/count;
            }
        }
    }

    return result;
}

template<size_t N>
std::vector<Rgb> blur_masked(const std::vector<Rgb>& img, const std::vector<OCTET>& mask, int h, int w, const std::array<std::array<bool,N>,N>& neighbors)
{
    const size_t neighbor_center = N/2;
    std::vector<Rgb> result = img;
    for(size_t i = 0; i < h; ++i)
    {
        for(size_t j = 0; j < w; ++j)
        {
            if(!mask[i*h+j]) continue;

            double sum_r = 0;
            double sum_g = 0;
            double sum_b = 0;
            unsigned count = 0;
            for(size_t k = 0; k < N; ++k)
            {
                if(k < neighbor_center && neighbor_center - k > i) continue;
                if(k > neighbor_center && h - (k - neighbor_center) <= i) continue;

                const ptrdiff_t k_offset = k - neighbor_center;
                for(size_t l = 0; l < N; ++l)
                {
                    if(!neighbors[k][l]) continue;
                    if(l < neighbor_center && neighbor_center - l > j) continue;
                    if(l > neighbor_center && w - (l - neighbor_center) <= j) continue;
                    const ptrdiff_t l_offset = l - neighbor_center;

                    sum_r += img[(i+k_offset)*h+j+l_offset].r;
                    sum_g += img[(i+k_offset)*h+j+l_offset].g;
                    sum_b += img[(i+k_offset)*h+j+l_offset].b;
                    ++count;
                }
            }

            if(count)
            {
                result[i*h+j].r = sum_r/count;
                result[i*h+j].g = sum_g/count;
                result[i*h+j].b = sum_b/count;
            }
        }
    }

    return result;
}

template<typename R, typename It>
void expand(R&& img, It it)
{
    OCTET min=255, max=0;
    for(OCTET o : img)
    {
        min = std::min(min, o);
        max = std::max(max, o);
    }
    double alpha = -255.*min/(max-min);
    double beta = 255./(max-min);
    std::cout << "Alpha : " << alpha << " beta : " << beta << std::endl;
    for(OCTET o : img)
    {
        *(it++) = alpha + o * beta;
    }
}
