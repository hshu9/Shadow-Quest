#include <iostream>
#include <string>
using namespace std;

string getRichQuick(int n) {
    if (n < 0) {
        return "";
    }
    
    string s = "$";
    
    for (int i = 0; i < n; i++) {
        s = s + s;
    }
    
    return s;
}

int main() {
    cout << getRichQuick(3) << endl;
    return 0;
}
