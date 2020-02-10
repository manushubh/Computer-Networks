#include <iostream>
using namespace std;

int main(int argc, const char * argv[]) {
    cout << "Enter size of buffer: ";
    int bufferSize;
    cin >> bufferSize;
    cout << "Enter output rate: ";
    int outputRate;
    cin >> outputRate;
    int choice;
    int dataSize;
    int bufferFilled = 0;
    while (true) {
        cout << "Is there any new data to send?" << endl << "Enter 1 for yes, 0 for no: ";
        cin >> choice;
        if (choice == 1) {
            cout << "Enter size of data: ";
            cin >> dataSize;
            if (dataSize > bufferSize - bufferFilled) {
                cout << "Packet dropped, insufficient space available in buffer";
            }
            else {
                cout << "Packet added to buffer" <<endl;
                bufferFilled += dataSize;
            }
        }
        if (bufferFilled != 0) {
            if (bufferFilled > outputRate) {
                cout << outputRate << " bytes sent" << endl;
                bufferFilled -= outputRate;
            }
            else {
                cout << bufferFilled << " bytes sent" << endl;
                bufferFilled = 0;
            }
        }
        else {
            cout << "No data to send, buffer empty" << endl;
        }
    }
    return 0;
}
