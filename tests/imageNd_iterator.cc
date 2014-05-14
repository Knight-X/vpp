#include <iostream>
#include <vpp/image2d.hh>

int main()
{
  using vpp::boxNd_iterator;
  using vpp::image2d;
  using vpp::vint2;


  image2d<int> img({3, 3});

  vint2 ref[] = {
    vint2(0, 0),
    vint2(0, 1),
    vint2(0, 2),
    vint2(1, 0),
    vint2(1, 1),
    vint2(1, 2),
    vint2(2, 0),
    vint2(2, 1),
    vint2(2, 2)
  };

  int i = 0;
  for (auto& p : img)
  {
    assert(&p == &img(ref[i]));
    i++;
  }
}
