#include<iostream>
#include<fstream>
#include<cstdlib>
#include<ctime>
#include<unordered_map>
#include<vector>
using namespace std;

unordered_map<string,int> counter;
vector<string> Index;
fstream fstr;
int n;

string getIndex(){
    while (1)
    {
        string tmp="";
        for (size_t i = 0; i < 5; i++)
        {
            tmp+=to_string(rand());
        }
        if (!counter.count(tmp))
        {
            return tmp;
        }
    }
}

int getCmd(int i){
    int k = rand()%1000;
    //insert new index
    if ((n>100 && i<10 || Index.size()<=4) || k<200) return 0;
    //insert old index with different value
    if (k<400) return 1;
    //can find
    if (k<780) return 2;
    //can not find
    if (k<900) return 3;
    return 4;
}

int main(){
    int k,kk;
    cin>>k>>kk;
    for (size_t kkk = k; kkk <= kk; kkk++)
    {  
        string file_name = to_string(kkk) + ".in";
        fstr.open(file_name,ios::out);
        cin>>n;
        srand(time(NULL));
        fstr<<n<<'\n';
        for (int i=0;i<n;++i){
            int cmd = getCmd(i);
            if (cmd==0)
            {
                string s = getIndex();
                counter[s] = 0;
                Index.push_back(s);
                fstr<<"insert "<<s<<' '<<0<<'\n';
            }
            else if (cmd == 1) {
                int k = rand()% Index.size();
                string s = Index[k];
                ++counter[s];
                fstr<<"insert "<<s<<' '<<counter[s]<<'\n';
            }
            else if (cmd == 2) {
                int k = rand()% Index.size();
                string s = Index[k];
                fstr<<"find "<<s<<'\n';
            } else if (cmd==3) {
                string s = getIndex();
                fstr<<"find "<<s<<'\n';
            } else {
                int k = rand()% Index.size();
                string s = Index[k];
                int v = rand()% (counter[s]+1);
                fstr<<"delete "<<s<<' '<<v<<'\n';
            }
        }
        fstr.close();
    }
}