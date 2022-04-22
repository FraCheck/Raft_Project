#ifndef UTILS_PRINTER_H_
#define UTILS_PRINTER_H_

#include <vector>

using namespace std;

string printVector(vector<int> v) {
    ostringstream out;
        out << "[ ";

        for (int i : v)
            out << i << " ";

        out << "]";
        return out.str();
}

string printElements(int *t, int size) {
    ostringstream out;
    out << "[ ";

    for (int i = 0; i < size; i++)
        out << t[i] << " ";

    out << "]";
    return out.str();
}

#endif
