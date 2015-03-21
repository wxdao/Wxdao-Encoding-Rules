#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

#include "wer.h"

using namespace std;
using namespace wer;

std::string formatbyte(byte *b, const size_t sz, const int base)
{
  string hexstr="";
  char tmp[3];
  for(unsigned int i=0;i<sz;i++) {
      memset(tmp,0,3);
      itoa(*(b+i),tmp,base);
      if(strlen(tmp)==1) {
          *(tmp+1)=*tmp;
          *tmp='0';
        }
      hexstr+=tmp;
    }
  return hexstr;
}

int main()
{
    cout << "Hello World!" << endl;
    wer_writer writer;
    wer_reader reader;
    writer.append_bin({1, 1}, (byte*)"haha", 5);
    writer.append_bin({1, 1}, (byte*)"haha", 5);
    writer.append_bin({1, 1}, (byte*)"haha", 5);
    writer.append_bin({1, 1}, (byte*)"haha", 5);
    wer_grp *grp = writer.append_grp({1, 1});
    writer.append_bin_to_grp(grp, {1, 1}, (byte*)"haha", 5);
    writer.generate();
    cerr << writer.len << endl;
    reader.load(writer.wer_data, writer.len);
    cerr << reader.read_all() << endl;
    cerr << reader.root->num << endl;
    cerr << ((wer_bin*)*(reader.root->subs.begin()))->parent_root << endl;
    wer_node_list list;
    cerr << formatbyte(writer.wer_data, writer.len, 16) << endl;
    list = reader.root->find_nodes_by_uid(1);
    cerr << list.size();
    delete reader.root;
    return 0;
}

