#include<iostream>
#include<fstream>
#include<vector>
#include"Unrolled_Linked_List.hpp"

using namespace std;
ULL::Unrolled_Linked_List<64> ull("a.file","b.file");
void create_file(std::string file_name) {
    std::fstream file(file_name, std::fstream::out);
    file.close();
}
int main(){
    //initialization
    std::fstream file("a.file", std::fstream::in);
    if (!file.is_open()) {
        create_file("a.file");
        create_file("b.file");
        ull.initialize(true);
    }
    else ull.initialize(false);
    int n;
    cin>>n;
    for (size_t i = 0; i < n; i++)
    {
        string s,index;
        int value;
        cin>>s>>index;
        if (s=="insert")
        {
            cin>>value;
            ull.insert(index.c_str(),value);
        } else if (s=="delete") {
            cin>>value;
            ull.Delete(index.c_str(),value);
        } else {
            vector<int> * vet = &ull.find(index.c_str());
            if (vet->empty())
            {
                cout<<"null\n";
            } else {
                for (size_t i = 0; i < vet->size(); i++)
                {
                    cout<<vet->operator[](i)<<((i==vet->size()-1)?'\n':' ');
                }
            }
            
            delete vet;
        }
    }
    
}