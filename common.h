#ifndef COMMON_H
#define COMMON_H

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

OCTET round_to_byte(double val);
OCTET round_clamped(double val);

OCTET round_offset(double val);

OCTET round_offset_clamped(double val);

double unoffset(OCTET val);

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

constexpr double sqr(double x);

constexpr double sqrDist(const Rgb& l, const Rgb& r);

template<typename T>
std::tuple<std::vector<T>, int, int> charger(std::string_view nom_image) requires std::same_as<T,Rgb> || std::same_as<T,OCTET>{
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
//std::tuple<std::vector<T>, int, int> charger(std::string_view nom_image);


void sauvegarder(std::string_view nom_image, const std::vector<Rgb>& img, int nb_lignes, int nb_colonnes);
void sauvegarder(std::string_view nom_image, const std::vector<OCTET>& img, int nb_lignes, int nb_colonnes);
double psnr(const std::vector<Rgb>& l, const std::vector<Rgb>& r);

template<size_t N>
constexpr std::array<std::array<bool,N>,N> makeDefaultNeighbors();

template<size_t N>
std::vector<OCTET> dilater_couleur(const std::vector<OCTET>& img, int h, int w, OCTET col, const std::array<std::array<bool,N>,N>& neighbors);

template<size_t N, typename F = std::identity>
std::vector<OCTET> filter(const std::vector<OCTET>& img, int h, int w, const std::array<std::array<double,N>,N>& f, F&& transformation = {});

template<size_t N>
std::vector<Rgb> blur(const std::vector<Rgb>& img, int h, int w, const std::array<std::array<bool,N>,N>& neighbors);

template<size_t N>
std::vector<Rgb> blur_masked(const std::vector<Rgb>& img, const std::vector<OCTET>& mask, int h, int w, const std::array<std::array<bool,N>,N>& neighbors);

template<typename R, typename It>
void expand(R&& img, It it);
#endif /* COMMON_H */
