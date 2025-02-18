#ifndef VPP_FILL_HH__
# define VPP_FILL_HH__

# include <vpp/core/boxNd.hh>
# include <vpp/core/vector.hh>
# include <vpp/core/parallel_for.hh>

namespace vpp
{

  template <typename V, typename U, unsigned N>
  void fill(imageNd<V, N>& img, U value)
  {
    pixel_wise(img) << [=] (auto& pix) { pix = value; };
  }

  template <typename V, unsigned N>
  void fill(imageNd<V, N>& img, V value, const boxNd<N>& box)
  {
    pixel_wise(box, img) << [=] (auto&, auto& pix) { pix = value; };
  }

  template <typename V, unsigned N>
  void fill_with_border(imageNd<V, N>& img, V value)
  {
    auto box = img.domain_with_border();
    pixel_wise(box, img) << [=] (auto&, auto& pix) { pix = value; };
  }

};

#endif
