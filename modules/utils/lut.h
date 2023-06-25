#include <vector>
#include <algorithm>

class LookUpTable {
private:
    std::vector<double> xs;
    std::vector<double> ys;
    std::vector<std::vector<double>> values;

public:
    LookUpTable(const std::vector<double>& xs, const std::vector<double>& ys, const std::vector<std::vector<double>>& values) 
        : xs(xs), ys(ys), values(values) {}

    double lookup2d(double x, double y) const {
        if(xs.empty() || ys.empty() || values.empty()) {
            return 0.0;
        }
        
        auto xit = std::lower_bound(xs.begin(), xs.end(), x);
        auto yit = std::lower_bound(ys.begin(), ys.end(), y);

        if (xit == xs.end()) {
            xit = xs.end() - 1;
        }
        if (yit == ys.end()) {
            yit = ys.end() - 1;
        }
        if (x < xs[0]) {
            xit = xs.begin();
        }
        if (y < ys[0]) {
            yit = ys.begin();
        }

        auto i = xit - xs.begin();
        auto j = yit - ys.begin();

        double x1 = xs[i];
        double x2 = xs[i + 1];
        double y1 = ys[j];
        double y2 = ys[j + 1];

        return (values[i][j] * (x2 - x) * (y2 - y) +
                values[i + 1][j] * (x - x1) * (y2 - y) +
                values[i][j + 1] * (x2 - x) * (y - y1) +
                values[i + 1][j + 1] * (x - x1) * (y - y1)) / ((x2 - x1) * (y2 - y1));
    }
};
