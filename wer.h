#ifndef WER_H
#define WER_H

#include <list>
#include <cstdlib>

namespace wer
{

typedef long len_t;
typedef unsigned char byte;

struct wer_node_appendix
{
    byte atr;
    long uid;
};

class wer_grp;
class wer_root;

class wer_node
{
public:
    len_t data_len;
    wer_node_appendix appendix;
    
    wer_grp *parent_grp;
    wer_root *parent_root;
    
    virtual ~wer_node();
    
    friend class wer_reader;
};

typedef std::list<wer_node*> wer_node_list;

class wer_bin : public wer_node
{
public:
    byte *data;
};

class wer_grp : public wer_node
{
public:
    long num;
    wer_node_list subs;
    
    wer_node_list find_nodes_by_uid(long uid);
    void clear();
    ~wer_grp();
};

class wer_root
{
public:
    long num;
    wer_node_list subs;
    
    wer_node_list find_nodes_by_uid(long uid);
    void clear();
    ~wer_root();
};

class wer_reader
{
    byte *wer_data;
    len_t len;
    len_t pos;
    wer_bin *read_bin();
    wer_grp *read_grp();
public:
    wer_reader(byte *_data, len_t _len);
    wer_reader();
    void load(byte *_data, len_t _len);
    len_t read_all();
    void clear();
    ~wer_reader();
    
    wer_root *root;
};

class wer_writer
{
    wer_root *root;
    len_t pos;
    
    void write_grp(wer_grp *);
    void write_bin(wer_bin *);
public:
    byte *wer_data;
    len_t len;
    
    wer_writer();
    ~wer_writer();
    void clear();
    void append_bin(wer_node_appendix _appendix, byte *_data, len_t _data_len);
    wer_grp *append_grp(wer_node_appendix _appendix);
    void append_bin_to_grp(wer_grp *_grp, wer_node_appendix _appendix, byte *_data, len_t _data_len);
    wer_grp *append_grp_to_grp(wer_grp *_grp, wer_node_appendix _appendix, byte *_data, len_t _data_len);
    bool generate();
};

} //namespace wer
#endif // WER_H

