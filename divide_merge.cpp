#include "divide_merge.h"

struct Region
{
    int top;
    int left;
    int height;
    int width;
    int total_height;
    int total_width;

    size_t operator()(int i, int j) const
    {
        return (top+i)*total_width+j+left;
    }

    std::array<Region, 4> subRegions() const
    {
        std::array<Region, 4> result={*this, *this, *this, *this};
        for(int i = 0; i < 4; ++i)
        {
            Region& region = result[i];
            region.height/=2;
            region.width/=2;
            if(i & 1) region.top+=region.height;
            if(i & 2) region.left+=region.width;
        }
        return result;
    }
};

struct Rag
{
    std::vector<Region> _regions;
    //For each index i, add a i-elements line that describes neighbor relationships with previous indexes
    //Therefore the ith line starts at the index (sum of numbers from 0 to i-1)=i*(i-1)/2
    std::vector<bool> _half_matrix;

    std::vector<size_t> neighborsOf(size_t index) const
    {
        std::vector<size_t> result;

        forHalfMatrixRow(_half_matrix, index, [&](size_t i, bool val){ if(val) result.push_back(i); });

        return result;
    }
    void deleteNeighbors(size_t index)
    {
        forHalfMatrixRow(_half_matrix, index, [&](size_t i, bool& val){ val=false; });
    }
    void addRegions(unsigned count)
    {
        size_t size = _regions.size() + count;
        _regions.resize(size);
        _half_matrix.resize(size*(size-1)/2, false);
    }
    void addEdge(size_t from, size_t to)
    {
        size_t line = std::max(from,to);
        _half_matrix[line*(line-1)/2+std::min(from,to)] = true;
    }
};

struct Mean
{
    float mean;
    size_t count;
};

DivideMerge::DivideMerge()
{

}


Region makeRegion(int h, int w)
{
    return Region{0,0,h,w,h,w};
}

bool areNeighbor(Region a, Region b)
{
    using std::min, std::max;
    const bool vertically_aligned = max(a.top, b.top) < min(a.top + a.height, b.top + b.height);
    const bool horizontally_aligned = max(a.left, b.left) < min(a.left + a.width, b.left + b.width);
    const bool common_row = a.top == b.top + b.height + 1 || b.top == a.top + a.height + 1;
    const bool common_col = a.left == b.left + b.width + 1 || b.left == a.left + a.width + 1;
    return vertically_aligned && common_row || horizontally_aligned && common_col;
}


template<typename T, typename F>
void forHalfMatrixRow(const std::vector<T>& half_matrix, size_t index, F&& func)
{
    size_t line_start = index*(index-1)/2;
    for(size_t i = 0; i < index; ++i)
    {
        func(i,half_matrix[line_start + i]);
    }
    size_t other_index = index+1;
    size_t other_start = line_start + index;
    while(other_start < half_matrix.size())
    {
        func(other_index, half_matrix[other_start + index]);
        other_start += other_index;
        ++other_index;
    }
}

template<typename T, typename F>
void forHalfMatrixRow(std::vector<T>& half_matrix, size_t index, F&& func)
{
    forHalfMatrixRow(std::as_const(half_matrix), index, [&](size_t i, const T& v){ func(i,const_cast<T&>(v)); });
}
template<typename ImgIn>
void buildRegions(const ImgIn& in, Rag& rag, std::vector<Mean>& means, size_t region_index, double max_variance)
{
    Region region = rag._regions[region_index];
    float sum = 0;
    float sqr_sum = 0;
    for(int i = 0; i < region.height; ++i)
    {
        for(int j = 0; j < region.width; ++j)
        {
            float val = in[region(i,j)];
            sum+=val;
            sqr_sum+=val*val;
        }
    }
    size_t count = (size_t)region.height * region.width;
    float mean = sum / count;
    float variance = sqr_sum / count - mean * mean;
    if(variance < max_variance || count <= 64)
    {
        means[region_index] = {mean, count};
        return;
    }

    std::vector<size_t> neighbors = rag.neighborsOf(region_index);
    rag.deleteNeighbors(region_index);
    auto subRegions = region.subRegions();
    size_t previous_last = rag._regions.size();
    rag.addRegions(3);
    means.resize(means.size() + 3);
    for(int i = 0; i < 4; ++i)
    {
        size_t index = i == 3 ? region_index : previous_last + i;
        rag._regions[index] = subRegions[i];
        for(size_t n_index : neighbors)
        {
            if(areNeighbor(rag._regions[index], rag._regions[n_index])) rag.addEdge(index, n_index);
        }
    }
    rag.addEdge(region_index, previous_last + 2);
    rag.addEdge(previous_last + 2, previous_last);
    rag.addEdge(previous_last, previous_last + 1);
    rag.addEdge(previous_last + 1, region_index);
    for(int i = 0; i < 4; ++i)
    {
        size_t index = i == 3 ? region_index : previous_last + i;
        buildRegions(in, rag, means, index, max_variance);
    }
}

std::tuple<float, size_t, size_t> findBestDist(const Rag& rag, const std::vector<Mean>& means, const std::vector<Mean>& group_means, const std::vector<size_t>& groups)
{
    float best_dist=255;
    size_t best_from, best_to;
    for(size_t i = 0; i < groups.size(); ++i)
    {
        float mean = groups[i] == 0 ? means[i].mean : group_means[groups[i]-1].mean;
        for(size_t other_i : rag.neighborsOf(i))
        {
            if(groups[i] == groups[other_i] && groups[i] != 0) continue;
            float other_mean = groups[other_i] == 0 ? means[other_i].mean : group_means[groups[other_i]-1].mean;
            float dist = std::abs(means[i].mean - means[other_i].mean);
            if(dist < best_dist)
            {
                best_dist = dist;
                best_from = i;
                best_to = other_i;
            }
        }
    }

    return std::tie(best_dist, best_from, best_to);
}
