#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <strstream>

template<class T>
alignment<T>::alignment(const T &s, const T &d, unsigned int source_offset, unsigned int dest_offset) 
    : S(s), 
    D(d),
    _source_offset(source_offset),
    _dest_offset(dest_offset)
{
    // ----------------------------------------
    // initialize V to zero's 
    // ----------------------------------------
    for(unsigned int i = 0; i <= S.size(); i++ ) 
    {
        vector<int> empty;
        V.push_back(empty);

        for(unsigned int j = 0; j <= D.size(); j++ ) 
        {
            V[i].push_back(0);
        }
        assert( V[i].size() == D.size()+1 );
    }
    assert( V.size() == S.size()+1 );
}

template<class T>
void
alignment<T>::find_optimal_alignment(bool hash_result) 
{
    if (hash_result) {
        unsigned int s1 = _source_offset, s2 = S.size() + _source_offset, d1= _dest_offset, d2 = D.size() + _dest_offset;
        if (0) {
        } else if (d2 == _dest_offset) {
            _entries.push_front(diff_entry(s1+1,s2,d1+1,d2, e_delete));
            return;
        } else if (s2 == _source_offset) {
            _entries.push_front(diff_entry(s1+1,s2,d1+1,d2, e_add));
            return;
        } else if (s2 == _source_offset + 1 && d2 == _dest_offset + 1) {
            _entries.push_front(diff_entry(s1+1,s2,d1+1,d2, e_change));
            return;
        }
    }

    for(unsigned int i = 1; i <= S.size(); i++ ) {
        V[i][0] = V[i-1][0] + S.score( S[i], S.space() );
    }

    for(unsigned int j = 1; j <= D.size(); j++ ) {
        V[0][j] = V[0][j-1] + D.score( D.space(), D[j] );
    }
    // ----------------------------------------
    // recurrence
    // ----------------------------------------
    
    for (unsigned int i = 1; i <= S.size(); i++ ) {
        for (unsigned int j = 1; j <= D.size(); j++ ) {
            int v = V[i-1][j-1] + S.score( S[i], D[j] );
            v = max( v, V[i-1][j] + S.score(S[i], D.space()) );
            v = max( v, V[i][j-1] + S.score(S.space(), D[j] ) );
            V[i][j] = v;
        }
    }

    // ----------------------------------------
    // input hash_result is a flag that will be 
    // set to true if the result should be hashed
    // ----------------------------------------
    if (hash_result)
    {
        unsigned int i = S.size();
        unsigned int j = D.size();

        diff_type type = e_none;

        unsigned int s1=i, s2=i, d1=j, d2=j;

        while ( i >0 || j > 0 ) 
        {
            int v = V[i][j];
            
            ostrstream ost;
            if ( i>0 && j >0 && v == V[i-1][j-1] + S.score( S[i], D[j] ) ) {
                if (type != e_change)
                {
                    s1 = i+1 + _source_offset;
                    d1 = j+1 + _dest_offset;
                    if (type != e_none) _entries.push_front(diff_entry(s1,s2,d1,d2,type));
                    s2 = i + _source_offset;
                    d2 = j + _dest_offset;
                    type = e_change;
                }
                i--;
                j--;
            } else if ( i>0 && v == V[i-1][j] + S.score(S[i], D.space()) ) {
                if (type != e_delete)
                {
                    s1 = i+1 + _source_offset;
                    d1 = j+1 + _dest_offset;
                    if (type != e_none) _entries.push_front(diff_entry(s1,s2,d1,d2,type));
                    s2 = i + _source_offset;
                    d2 = j + _dest_offset;
                    type = e_delete;
                }
                i--;
            } else {
                assert( v == V[i][j-1] + S.score(S.space(), D[j] )) ;
                if (type != e_add)
                {
                    s1 = i+1 + _source_offset;
                    d1 = j+1 + _dest_offset;
                    if (type != e_none) _entries.push_front(diff_entry(s1,s2,d1,d2,type));
                    s2 = i + _source_offset;
                    d2 = j + _dest_offset;
                    type = e_add;
                }
                j--;
            }
        }
        s1 = i+1 + _source_offset;
        d1 = j+1 + _dest_offset;
        if (type != e_none) _entries.push_front(diff_entry(s1,s2,d1,d2,type));
    }
}

template<class T>
ostream &
alignment<T>::dump(ostream & os)  const
{
    os << "..dump.." << endl;
    for (unsigned int i = 0; i <= S.size(); i++ ) {
        for (unsigned int j = 0; j <= D.size(); j++ ) {
            os << V[i][j] << " ";
        }
        os << endl;
    }
    return os;
}

template<class T>
int
alignment<T>::optimal_alignment_value()  const
{
    return V[ S.size()][ D.size() ];
}

template<class T>
istream &
alignment<T>::read(istream & is)
{
    return is;
}

template<class T>
ostream &
alignment<T>::show(ostream & os) const
{
    return    diff2(os);
// return diff1(os);
}

template<class T>
ostream &
alignment<T>::diff_output(ostream & os, 
                          unsigned int s1, 
                          unsigned int s2,
                          unsigned int d1,
                          unsigned int d2,
                          diff_type type) const
{
    if (type == e_null)
    {
        return os;
    }

    if (type == e_add)
    {
        s1 = s2;
    } else if (type == e_delete)
    {
        d1 = d2;
    }

    os << s1 + _source_offset;
    if (s1 != s2)
    {
        os << "," << s2 + _source_offset;
    }
    os << type;
    os << d1 + _dest_offset;
    if (d1 != d2)
    {
        os << "," << d2 + _dest_offset;
    }
    os << endl;
    return os;
}

template<class T>
ostream &
alignment<T>::diff2(ostream & os) const
{
    for (list<diff_entry>::const_iterator is = _entries.begin(); is != _entries.end(); ++is) 
    {
        os << (*is) << endl;
    }
    return os;
}

template<class T>
ostream &
alignment<T>::diff1(ostream & os) const
{
    // ----------------------------------------
    // comes up with one alignment, not necessarily
    // all alignments
    // ----------------------------------------
    {
        unsigned int max_length = 0;
        unsigned int i = S.size();
        unsigned int j = D.size();

        if (j <= 1)
        {
            return os;
        }

        list<string> s;
        list<string> t;

        while ( i >0 || j > 0 ) {

            int v = V[i][j];
            string line1;
            string line2;
            ostrstream ost1;
            ostrstream ost2;

            if ( i>0 && j >0 && v == V[i-1][j-1] + S.score( S[i], D[j] ) ) {
                ost1 << i + _source_offset << " " << S[i--] << ends;
                ost2 << j + _dest_offset << " " << D[j--] << ends;
                line1 = ost1.str();
                line2 = ost2.str();
                max_length = (max_length > line1.length()) ? max_length : line1.length();
                s.push_front( ost1.str());
                t.push_front( ost2.str());
            } else if ( i>0 && v == V[i-1][j] + S.score(S[i], D.space()) ) {
                ost1 << i + _source_offset << " " << S[i--] << ends;
                ost2 << j + _dest_offset << " " << "$" << ends;
                line1 = ost1.str();
                line2 = ost2.str();
                max_length = (max_length > line1.length()) ? max_length : line1.length();
                s.push_front( ost1.str());
                t.push_front( ost2.str());
            } else {
                assert( v == V[i][j-1] + S.score(S.space(), D[j] )) ;
                ost1 << i + _source_offset << " " << "$" << ends;
                ost2 << j + _dest_offset << " " << D[j--] << ends;
                line1 = ost1.str();
                line2 = ost2.str();
                max_length = (max_length > line1.length()) ? max_length : line1.length();

                s.push_front( ost1.str());
                t.push_front( ost2.str());
            }

        }
        assert( s.size() == t.size() );

        list<string>::iterator is = s.begin();
        list<string>::iterator it = t.begin();
        for (unsigned int i = 0; i < s.size(); i++ ) {
            os << (*is);
            for (unsigned int j = 0; j < max_length - (*is).length(); ++j)
            {
                os << " ";
            }
            os << "|" << (*it) << endl;
            is++;
            it++;
        }
    }
    return os;
}
