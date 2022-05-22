#include "generategrid.h"

GenerateGrid::GenerateGrid(int width, int height)
{
    m_width = width;
    m_height = height;
}

//------------------------------------------------------------------------------------------
QDebug operator<< (QDebug d, const Vec2 &vec2) {
    d << " ("<<vec2.x<<", "<<vec2.y<<") ";
    return d;
}
QDebug operator<< (QDebug d, const Triangle &triangle) {
    d << " "<<triangle.a<<"-"<<triangle.b<<"-"<<triangle.c <<" ";
    return d;
}

// Function to print the
// index of an element
int getVertexIndex(std::vector<Vec2> v, Vec2 vertex)
{
    auto it = find(v.begin(), v.end(), vertex);

    // If element was found
    if (it != v.end())
    {
        return it - v.begin() ;
    }
    else {
        // If the element is not present in the vector
        return -1;
    }
}

void GenerateGrid::echantillonageContour(std::vector<unsigned char>& edgeMap, int seuil, int maxPoints, float pointRate){
    int sum, total;
    int x, y, sx, sy   ;
    int row, col, step ;
    std::vector<Vec2> points;


    for (y = 0; y < m_height; y++) {
        for (x = 0; x < m_width; x++) {
            sum = 0;
            total =0;

            for (row = -1; row <= 1; row++) {
                sy = y + row;
                step = sy * m_width;

                if (sy >= 0 && sy < m_height) {
                    for (col = -1; col <= 1; col++) {
                        sx = x + col;
                        if (sx >= 0 && sx < m_width) {
                            sum += (int)edgeMap[(sx+step)*4];
                            total++;
                        }
                    }
                }
            }
            if (total > 0) {
                sum /= total;
            }
            if (sum > seuil) {
                points.push_back({(float)x, (float)y});
            }
        }
    }

    qDebug()<< points[0]<< Qt::endl;

    int ilen = points.size();
    int tlen = ilen;
    int limit = (int)((float)ilen * pointRate);

    if (limit > maxPoints) {
        limit = maxPoints;
    }


    for (int i = 0; i < limit && i < ilen; i++) {
        // j = int(float64(tlen) * rand.Float64())
        //random float number between 0 and 1
        float rand_number = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        int j = (int)((float)(tlen) * rand_number);

    //            m_vertices.push_back(points[j]);
        Vec2 point={points[j].x/(float)m_height, points[j].y/(float)m_width};


        m_vertices.push_back(point);

//        qDebug()<<"point ="<<point<< Qt::endl;
        // Remove points
        // points = append(points[:j], points[j+1:]...)
        std::erase(points, points[j]);
        tlen--;
    }
    // Ajout des 4 coins
    if (getVertexIndex(m_vertices, {0.0f, 0.0f}) == -1){
        m_vertices.push_back({0.0f, 0.0f});
    }
    if (getVertexIndex(m_vertices, {1.0f, 0.0f}) == -1){
        m_vertices.push_back({1.0f, 0.0f});
    }
    if (getVertexIndex(m_vertices, {0.0f, 1.0f}) == -1){
        m_vertices.push_back({0.0f, 1.0f});
    }
    if (getVertexIndex(m_vertices, {1.0f, 1.0f}) == -1){
        m_vertices.push_back({1.0f, 1.0f});
    }
}

void GenerateGrid::delaunayTriangulation(){
    std::vector<double> coords = vectorOfVec2TovectorOfDouble(m_vertices);
    Delaunator d(coords);

    for(std::size_t i = 0; i < d.triangles.size(); i+=3) {
        Vec2 a = {(float)d.coords[2 * d.triangles[i]], (float)d.coords[2 * d.triangles[i] + 1]};
        Vec2 b = {(float)d.coords[2 * d.triangles[i + 1]], (float)d.coords[2 * d.triangles[i + 1] + 1]};
        Vec2 c = {(float)d.coords[2 * d.triangles[i + 2]], (float)d.coords[2 * d.triangles[i + 2] + 1]};
        int a_index = getVertexIndex(m_vertices, a);
        int b_index= getVertexIndex(m_vertices, b);
        int c_index= getVertexIndex(m_vertices, c);
        if (a_index == -1 || b_index== -1 || c_index== -1 )
        {
            qDebug()<< a_index<< b_index<<c_index << Qt::endl;
            exit(3);
        }
        else
        {
            m_triangles.push_back({(unsigned int)a_index, (unsigned int)b_index, (unsigned int)c_index});
        }
    }
}
//------------------------------------------------------------------------------------------
void GenerateGrid::computeTriangulationGradientMap(QString filename, int seuil, int maxPoints, float pointRate){
    // Génération carte de Gradient
    std::string sringFilename = filename.toStdString();
    auto [img, nH, nW] = charger<OCTET>(sringFilename);
    std::vector<unsigned char> copy = img;

    float h, v;
    unsigned int G;
    for (int i = 0; i < nH; i++)
    {
        for (int j = 0; j < nW; j++)
        {
            if (i == nH - 1)
                h = 0;
            if (j == nW - 1)
                v = 0;
            if (i < nH - 1 && j < nW - 1)
            {
                h = (float)img[i * nW + (j + 1)] - (float)img[i * nW + j];
                // h = (float)img(i, j + 1) - (float)img(i, j);
                v = (float)img[(i + 1) * nW + j] - (float)img[i * nW + j];
                // v = (float)img(i + 1, j) - (float)img(i, j);

                G = (unsigned int)std::min(255.0f, std::sqrt(h * h + v * v));
                copy[i * nW + j] = G;
            }
        }
    }
    sauvegarder("gradient.pgm", copy, nH, nW);

    //Récupérer vertices
    echantillonageContour(copy, seuil, maxPoints, pointRate);
    // qDebug()<<"m_vertices.size() ="<<m_vertices.size()<< Qt::endl;

    //Delaunay Triangulation
    delaunayTriangulation();
}

void GenerateGrid::computeTriangulationSobelMap(QString filename, int seuilFiltre, int seuil, int maxPoints, float pointRate){
    // Génération carte de Sobel
    std::string sringFilename = filename.toStdString();
    auto [img, nH, nW] = charger<OCTET>(sringFilename);
//    std::vector<unsigned char> copy = img;
    std::vector<unsigned char> copy(m_width*m_height);

    int sobelKernelX [3][3] ={
            {-1, 0, 1},
            {-2, 0, 2},
            {-1, 0, 1},
        };

    int sobelKernelY [3][3] ={
            {-1, -2, -1},
            {0, 0, 0},
            {1, 2, 1},
        };


    int sumX, sumY;

    // Get 3x3 window of pixels because image data given is just a 1D array of pixels
    // 3 = len(sobelKernelX)
    int maxPixelOffset = m_width*2 + 3 - 1;

    // len(img) = (m_width*m_height)
    int length = (m_width*m_height)*4 - maxPixelOffset;
    std::vector<unsigned int> magnitudes;
    magnitudes.resize(length);
//    uint magnitudes[length];

    for (int i = 0; i < length; i++) {
        // Sum each pixel with the kernel value
        sumX = 0;
        sumY = 0;

        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                int idx = i + (m_width * y) + x;
                if  (idx < (m_width*m_height)) {
                    int r = img[i+(m_width*y)+x];
                    sumX += r * sobelKernelX[y][x];
                    sumY += r * sobelKernelY[y][x];
                }
            }
        }
        float magnitude = std::sqrt((float)(sumX*sumX) + (float)(sumY*sumY));
        // Check for pixel color boundaries
        if (magnitude < 0) {
            magnitude = 0;
        } else if (magnitude > 255) {
            magnitude = 255;
        }

        // Set magnitude to 0 if doesn't exceed threshold, else set to magnitude
        if (magnitude > seuilFiltre) {
            magnitudes[i] = (unsigned int)magnitude;
        } else {
            magnitudes[i] = 0;
        }
    }

    int dataLength = m_width * m_height * 4;
//    int edges [dataLength] ;
    std::vector<int> edges;
    edges.resize(dataLength);

    // Apply the kernel values
    for (int i = 0; i < dataLength; i++){
        edges[i] = 0;
        if (i%4 != 0) {
            int m = magnitudes[i/4];
            if (m != 0) {
                edges[i-1] = m;
            }
        }
//        std::cout << edges[i] << std::endl;
    }

    // Generate the new image with the sobel filter applied
//    std::cout << edges.size() << std::endl;

    int i =0;
    for (size_t idx = 0; idx < edges.size(); idx += 4) {
//        std::cout << edges[idx] << std::endl;
        copy[i] = (unsigned int)edges[idx];
        i++;
//        copy[idx+1] = (unsigned int)edges[idx+1];
//        copy[idx+2] = (unsigned int)edges[idx+2];
//        copy[idx+3] = 255;
    }

    sauvegarder("sobel.pgm", copy, nH, nW);
    //----------------------------------------
    //Récupérer vertices
    echantillonageContour(copy, seuil, maxPoints, pointRate);

    // qDebug()<<"m_vertices.size() ="<<m_vertices.size()<< Qt::endl;

    //Delaunay Triangulation
    delaunayTriangulation();
}
//------------------------------------------------------------------------------------------
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
void GenerateGrid::computeTriangulationSplitAndMerge(QString filename,double maxVariance,int maxDist)
{
        //----------------------------------------
        //Split and Merge
        std::string sringFilename = filename.toStdString();
        auto [img, h, w] = charger<OCTET>(sringFilename);

        auto copy = img;

        Rag rag {{makeRegion(h,w)},{}};
        std::vector<Mean> means(1);
        buildRegions(img, rag, means, 0, maxVariance);
        int max_dist = maxDist;
        std::vector<size_t> groups(means.size(),0);
        std::vector<Mean> group_means;

        float dist;
        size_t from, to;
        /*
        while(std::tie(dist,from,to) = findBestDist(rag, means, group_means, groups), dist < max_dist){
        /*/
        bool did_something = true;
        bool oldval;
        while(oldval = did_something, did_something = false, oldval) for(size_t i = 0; i < groups.size(); ++i) for(size_t other_i : rag.neighborsOf(i))
        {
            if(groups[i] == groups[other_i] && groups[i] != 0) continue;
            float mean = groups[i] == 0 ? means[i].mean : group_means[groups[i]-1].mean;
            float other_mean = groups[other_i] == 0 ? means[other_i].mean : group_means[groups[other_i]-1].mean;
            dist = std::abs(means[i].mean - means[other_i].mean);
            if(dist > max_dist) continue;
            from = i; to = other_i;
            did_something = true;//*/

//            std::cout << "Distance between group " << from << " and " << to << " is " << dist << std::endl;
            if(groups[to] != 0) std::swap(from,to);
            if(size_t other_group = groups[to]; other_group != 0)
            {
                for(size_t& g : groups) if(g==other_group) g = groups[from];
                Mean& mean = group_means[groups[from]-1];
                Mean& other_mean = group_means[other_group-1];
                mean.mean = mean.mean * mean.count + other_mean.mean * other_mean.count;
                mean.count += other_mean.count;
                mean.mean /= mean.count;
            }
            else
            {
                if(groups[from] == 0)
                {
                    groups[from] = group_means.size()+1;
                    group_means.push_back(means[from]);
                }
                groups[to] = groups[from];
                Mean& mean = group_means[groups[from]-1];
                Mean& other_mean = means[to];
                mean.mean = mean.mean * mean.count + other_mean.mean * other_mean.count;
                mean.count += other_mean.count;
                mean.mean /= mean.count;
            }
//            std::cout << "Iteration done" << std::endl;
        }

        for(size_t i = 0; i < rag._regions.size(); ++i)
        {
            Region region = rag._regions[i];
            float mean = (groups[i] == 0 ? means[i] : group_means[groups[i]-1]).mean;
            for(int j = 0; j < region.height; ++j)
            {
                for(int k = 0; k < region.width; ++k)
                {
                    copy[region(j,k)] = mean;
                }
            }
        }

//        size_t lastindex = sringFilename.find_last_of("/");
//        std::string rawname = sringFilename.substr(0, lastindex);
//        qDebug()<< QString::fromStdString(rawname) <<Qt::endl;

        sauvegarder("split.pgm", copy, h, w);

        //----------------------------------------
        //Récupérer vertices
        unsigned int k = 0;
        int a_index, b_index, c_index, d_index;
        Vec2 A, B, C, D;
        Triangle t1, t2;
        for (size_t i = 0; i < rag._regions.size(); i++)
        {
            Region region = rag._regions[i];

            A = {(float)region.left/(float)m_width,
                 (float)((float)m_height -region.top)/(float)m_height};
            B= {(float)(region.left + region.width)/(float)m_width,
                (float)((float)m_height -region.top)/(float)m_height};
            C= {(float)region.left/(float)m_width,
                (float)(((float)m_height -region.top)- region.height)/(float)m_height};
            D= {(float)(region.left+ region.width)/(float)m_width,
                (float)(((float)m_height -region.top)- region.height)/(float)m_height};

            a_index =getVertexIndex(m_vertices, A);
            if (a_index == -1){
//                qDebug()<<"A not found"<< Qt::endl;
                m_vertices.push_back(A);
                a_index = k;
                k++;
            }
            b_index =getVertexIndex(m_vertices, B);
            if (b_index == -1){
//                qDebug()<<"B not found"<< Qt::endl;
                m_vertices.push_back(B);
                b_index = k;
                k++;
            }
            c_index =getVertexIndex(m_vertices, C);
            if (c_index == -1){
//                qDebug()<<"C not found"<< Qt::endl;
                m_vertices.push_back(C);
                c_index = k;
                k++;
            }
            d_index =getVertexIndex(m_vertices, D);
            if (d_index == -1){
//                qDebug()<<"D not found"<< Qt::endl;
                m_vertices.push_back(D);
                d_index = k;
                k++;
            }

            t1 = {(unsigned int)a_index, (unsigned int)c_index, (unsigned int)d_index};
            t2 = {(unsigned int)a_index, (unsigned int)d_index, (unsigned int)b_index};
//            qDebug()<<A<<B<<C<<D;
//            qDebug()<<t1 <<t2<<Qt::endl;
        }

        //----------------------------------------
        //Delaunay Triangulation
        delaunayTriangulation();

}

//------------------------------------------------------------------------------------------
std::vector<double> GenerateGrid::vectorOfVec2TovectorOfDouble(std::vector<Vec2> vectorOfVec2)
{
    std::vector<double> vectorOfDouble;
    vectorOfDouble.reserve(2*vectorOfVec2.size());

    for (size_t i =0; i< vectorOfVec2.size(); i++)
    {
        vectorOfDouble.push_back((double)vectorOfVec2[i].x);
        vectorOfDouble.push_back((double)vectorOfVec2[i].y);
    }
    return vectorOfDouble;
}
std::vector<Vec2> GenerateGrid::getVertices()
{
    return m_vertices;
}
std::vector<Triangle> GenerateGrid::getTriangles()
{
    return m_triangles;
}
