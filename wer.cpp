#include <cstring>
#include <typeinfo>
#include "wer.h"

#include <iostream>
#define DEBUG(x) std::cerr << x << std::endl;

using namespace wer;
using namespace std;


#define overflow(t) ((pos + t > len) ? true : false)
#define overflow2(t, _len) ((pos + t > _len) ? true : false)
#define cur_buf (wer_data + pos)

wer_node::~wer_node()
{

}

void wer_grp::clear()
{
    for (wer_node_list::iterator it = subs.begin(); it != subs.end(); it++)
    {
        delete (*it);
    }
    subs.clear();
}

wer_grp::~wer_grp()
{
    clear();
}

wer_node_list wer_grp::find_nodes_by_uid(long uid)
{
    wer_node_list list;
    for (wer_node_list::iterator it = subs.begin(); it != subs.end(); it++)
    {
        if (((wer_node*)*it)->appendix.uid == uid)
        {
            list.push_back(*it);
            if (dynamic_cast<wer_grp*>(*it))
            {
                wer_node_list tmp_list = ((wer_grp*)*it)->find_nodes_by_uid(uid);
                for (wer_node_list::iterator it2 = tmp_list.begin(); it2 != tmp_list.end(); it2++)
                {
                    list.push_back(*it2);
                    
                }
            }
        }
    }
    return list;
}

wer_node_list wer_root::find_nodes_by_uid(long uid)
{
    wer_node_list list;
    for (wer_node_list::iterator it = subs.begin(); it != subs.end(); it++)
    {
        if (((wer_node*)*it)->appendix.uid == uid)
        {
            list.push_back(*it);
            if (dynamic_cast<wer_grp*>(*it))
            {
                wer_node_list tmp_list = ((wer_grp*)*it)->find_nodes_by_uid(uid);
                for (wer_node_list::iterator it2 = tmp_list.begin(); it2 != tmp_list.end(); it2++)
                {
                    list.push_back(*it2);
                }
            }
        }
    }
    return list;
}

void wer_root::clear()
{
    for (wer_node_list::iterator it = subs.begin(); it != subs.end(); it++)
    {
        delete (*it);
    }
}

wer_root::~wer_root()
{
    
    clear();
}

wer_reader::wer_reader()
{
    root = 0;
    
}

void wer_reader::load(byte *_data, len_t _len)
{
    wer_data = _data;
    pos = 0;
    len = _len;
    
}

wer_reader::wer_reader(byte *_data, len_t _len)
{
    root = 0;
    load(_data, _len);
}

wer_bin *wer_reader::read_bin()
{
    wer_bin *wb = new wer_bin();
    pos += 1; // jump over the identifier
    if (overflow(9))
    {
        delete (wb);
        return 0;
    }
    memcpy(&(wb->appendix.atr), cur_buf, 1);
    pos += 1;
    memcpy(&(wb->appendix.uid), cur_buf, 4);
    pos += 4;
    memcpy(&(wb->data_len), cur_buf, 4);
    pos += 4;
    
    if (overflow(wb->data_len))
    {
        delete (wb);
        return 0;
    }
    wb->data = cur_buf;
    pos += wb->data_len;
    return wb;
}

wer_grp *wer_reader::read_grp()
{
    wer_grp *wg = new wer_grp();
    pos += 1; // jump over the identifier
    if (overflow(9))
    {
        delete (wg);
        return 0;
    }
    memcpy(&(wg->appendix.atr), cur_buf, 1);
    pos += 1;
    memcpy(&(wg->appendix.uid), cur_buf, 4);
    pos += 4;
    memcpy(&(wg->data_len), cur_buf, 4);
    pos += 4;
    if (overflow(wg->data_len))
    {
        delete (wg);
        return 0;
    }
    len_t own_len = pos + wg->data_len;
    while (1)
    {
        if (overflow2(1,own_len))
        {
            return wg;
        }
        if (*cur_buf == 0x81)
        {
            wer_bin *wb = read_bin();
            if (wb)
            {
                wb->parent_grp = wg;
                wb->parent_root = root;
                wg->subs.push_back(wb);
                wg->num++;
            }
            else
            {
                delete (wg);
                return 0;
            }
        }
        else if(*cur_buf == 0x01)
        {
            wer_grp *_wg = read_grp();
            if (_wg)
            {
                _wg->parent_grp = wg;
                _wg->parent_root = root;
                wg->subs.push_back(_wg);
                wg->num++;
            }
            else
            {
                delete (wg);
                return 0;
            }
        }
        else
        {
            delete (wg);
            return 0;
        }
    }
}

len_t wer_reader::read_all()
{
    if (root)
    {
        delete (root);
    }
    
    root = new wer_root();
    len_t cur_pos;
    pos = 0;
    while (1)
    {
        if (overflow(1))
        {
            return pos;
        }
        if (*cur_buf == 0x81)
        {
            cur_pos = pos;
            wer_bin *wb = read_bin();
            if (wb)
            {
                wb->parent_grp = 0;
                wb->parent_root = root;
                root->subs.push_back((wer_node*)wb);
                root->num++;
            }
            else
            {
                return cur_pos;
            }
        }
        else if(*cur_buf == 0x01)
        {
            cur_pos = pos;
            wer_grp *wg = read_grp();
            if (wg)
            {
                wg->parent_grp = 0;
                wg->parent_root = root;
                root->subs.push_back(wg);
                root->num++;
            }
            else
            {
                return cur_pos;
            }
        }
        else
        {
            return pos;
        }
    }
}

wer_reader::~wer_reader()
{
    
}


wer_writer::wer_writer()
{
    wer_data = 0;
    len = 0;
    root = 0;
    pos = 0;
}

wer_writer::~wer_writer()
{
    clear();
}

void wer_writer::clear()
{
    delete (root);
    root = 0;
    if (wer_data)
    {
        delete (wer_data);
    }
    wer_data = 0;
}

void wer_writer::append_bin(wer_node_appendix _appendix, byte *_data, len_t _data_len)
{
    if (!root)
    {
        root = new wer_root();
    }
    
    wer_bin *wb = new wer_bin();
    wb->appendix = _appendix;
    wb->data = _data;
    wb->data_len = _data_len;
    
    root->subs.push_back(wb);
}

wer_grp *wer_writer::append_grp(wer_node_appendix _appendix)
{
    if (!root)
    {
        root = new wer_root();
    }
    
    wer_grp *wg = new wer_grp();
    wg->appendix = _appendix;
    
    root->subs.push_back(wg);
    return wg;
}

void wer_writer::append_bin_to_grp(wer_grp *_grp, wer_node_appendix _appendix, byte *_data, len_t _data_len)
{
    wer_bin *wb = new wer_bin();
    wb->appendix = _appendix;
    wb->data = _data;
    wb->data_len = _data_len;
    
    _grp->subs.push_back(wb);
}

wer_grp *wer_writer::append_grp_to_grp(wer_grp *_grp, wer_node_appendix _appendix, byte *_data, len_t _data_len)
{
    wer_grp *wg = new wer_grp();
    wg->appendix = _appendix;
    
    _grp->subs.push_back(wg);
    return wg;
}

len_t get_grp_size(wer_grp *grp)
{
    len_t total = 0;
    total += 10;
    for (wer_node_list::iterator it = grp->subs.begin(); it != grp->subs.end(); it++)
    {
        if (dynamic_cast<wer_bin*>(*it))
        {
            total += 10 + ((wer_bin*)(*it))->data_len;
        }
        else if (dynamic_cast<wer_grp*>(*it))
        {
            total += get_grp_size((wer_grp*)*it);
        }
    }
    return total;
}

void wer_writer::write_bin(wer_bin *wb)
{
    *cur_buf = 0x81;
    pos++;
    memcpy(cur_buf, &wb->appendix.atr, 1);
    pos += 1;
    memcpy(cur_buf, &wb->appendix.uid, 4);
    pos += 4;
    memcpy(cur_buf, &wb->data_len, 4);
    pos += 4;
    memcpy(cur_buf, wb->data, wb->data_len);
    pos += wb->data_len;
}

void wer_writer::write_grp(wer_grp *wg)
{
    *cur_buf = 0x01;
    pos++;
    memcpy(cur_buf, &wg->appendix.atr, 1);
    pos += 1;
    memcpy(cur_buf, &wg->appendix.uid, 4);
    pos += 4;
    wg->data_len = get_grp_size(wg) - 10;
    memcpy(cur_buf, &wg->data_len, 4);
    pos += 4;
    for (wer_node_list::iterator it = wg->subs.begin(); it != wg->subs.end(); it++)
    {
        if (dynamic_cast<wer_bin*>(*it))
        {
            write_bin((wer_bin*)(*it));
        }
        else if (dynamic_cast<wer_grp*>(*it))
        {
            write_grp((wer_grp*)(*it));
        }
    }
}

bool wer_writer::generate()
{
    //first we determine the total size needed
    len = 0;
    for (wer_node_list::iterator it = root->subs.begin(); it != root->subs.end(); it++)
    {       
        if (dynamic_cast<wer_bin*>(*it))
        {
            len += 10 + ((wer_bin*)(*it))->data_len;
        }
        else if (dynamic_cast<wer_grp*>(*it))
        {
            len += get_grp_size((wer_grp*)*it);
        }
    }
    
    if (wer_data)
    {
        delete (wer_data);
    }
    wer_data = new byte[len];
    
    //then we start to write them
    pos = 0;
    for (wer_node_list::iterator it = root->subs.begin(); it != root->subs.end(); it++)
    {
        if (dynamic_cast<wer_bin*>(*it))
        {
            write_bin((wer_bin*)(*it));
        }
        else if (dynamic_cast<wer_grp*>(*it))
        {
            write_grp((wer_grp*)(*it));
        }
    }
}


