%{
/* om_util_perl5.i: the Open Muscat scripting interface helpers.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
 * 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */
%}
%include typemaps.i

%typemap(perl5, out) string {
    $target = sv_newmortal();
    sv_setpv($target, ($source)->c_str());
    argvi++;
    delete $source;
    $source = 0;
}

%typemap(perl5, in) const string &(string temp) {
    if (SvPOK($source)) {
        STRLEN slen;
        char *ctemp = SvPV($source, slen);
	temp = string(ctemp, slen);
	$target = &temp;
    } else {
        croak("Type error: expected string.");
    }
}

%typemap(perl5, in) const vector<OmQuery *> *(vector<OmQuery *> v){
    if (!SvROK($source)) { // check that it's a reference...
        croak("Expected a reference");
    }
    if (SvTYPE(SvRV($source)) != SVt_PVAV) { // ... to an array
        croak("Expected an array reference");
    }

    AV *tempav = (AV *)SvRV($source);
    I32 len = av_len(tempav);
    for (int i=0; i<=len; ++i) {
        SV **sv = av_fetch(tempav, i, 0);
	OmQuery *qp;
	if (SWIG_GetPtr(*sv, (void **)&qp, "OmQuery")) {
	    croak("Reference invalid - expected OmQuery.");
	} else {
	    v.push_back(qp);
	}
    }
    $target = &v;
}

%typemap(perl5, out) om_termname_list {
    int len = $source->size();
    SV **svs = new SV *[len];

    om_termname_list::const_iterator tn = $source->begin();
    for (int i=0; i<len; ++tn, ++i) {
        svs[i] = sv_newmortal();
	sv_setpvn((SV*)svs[i], tn->c_str(), tn->length());
    }
    AV *myav = av_make(len, svs);

    //delete [] svs;
    //delete $source;
    $source = 0;

    $target = newRV((SV *)myav);
    sv_2mortal($target);
    argvi++;
}
