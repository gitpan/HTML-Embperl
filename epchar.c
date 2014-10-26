/*###################################################################################
#
#   Embperl - Copyright (c) 1997-2001 Gerald Richter / ECOS
#
#   You may distribute under the terms of either the GNU General Public
#   License or the Artistic License, as specified in the Perl README file.
#   For use with Apache httpd and mod_perl, see also Apache copyright.
#
#   THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
#   IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
#   WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
#   $Id: epchar.c,v 1.12.4.6 2001/11/02 11:34:26 richter Exp $
#
###################################################################################*/

/* input and output escaping for iso-8859-1 (iso-latin-1) */

#include "ep.h"



/*
* Character Translation
*/


struct tCharTrans Char2Html [] = 

    {
        { ' ' ,   ""         },    /* &#00;		Unused */ 
        { ' ' ,   ""         },    /* &#01;		Unused */
        { ' ' ,   ""         },    /* &#02;		Unused  */
        { ' ' ,   ""         },    /* &#03;		Unused  */
        { ' ' ,   ""         },    /* &#04;		Unused  */
        { ' ' ,   ""         },    /* &#05;		Unused  */
        { ' ' ,   ""         },    /* &#06;		Unused  */
        { ' ' ,   ""         },    /* &#07;		Unused  */
        { ' ' ,   ""         },    /* &#08;		Unused  */
        { ' ' ,   ""         },    /* &#09;		Horizontal tab  */
        { ' ' ,   ""         },    /* &#10;		Line feed  */
        { ' ' ,   ""         },    /* &#11;		Unused  */
        { ' ' ,   ""         },    /* &#12;		Unused  */
        { ' ' ,   ""         },    /* &#13;		Carriage Return  */
        { ' ' ,   ""         },    /* &#14;		Unused  */
        { ' ' ,   ""         },    /* &#15;		Unused  */
        { ' ' ,   ""         },    /* &#16;		Unused  */
        { ' ' ,   ""         },    /* &#17;		Unused  */
        { ' ' ,   ""         },    /* &#18;		Unused  */
        { ' ' ,   ""         },    /* &#19;		Unused  */
        { ' ' ,   ""         },    /* &#20;		Unused  */
        { ' ' ,   ""         },    /* &#21;		Unused  */
        { ' ' ,   ""         },    /* &#22;		Unused  */
        { ' ' ,   ""         },    /* &#23;		Unused  */
        { ' ' ,   ""         },    /* &#24;		Unused  */
        { ' ' ,   ""         },    /* &#25;		Unused  */
        { ' ' ,   ""         },    /* &#26;		Unused  */
        { ' ' ,   ""         },    /* &#27;		Unused  */
        { ' ' ,   ""         },    /* &#28;		Unused  */
        { ' ' ,   ""         },    /* &#29;		Unused  */
        { ' ' ,   ""         },    /* &#30;		Unused  */
        { ' ' ,   ""         },    /* &#31;		Unused  */
        { ' ' ,   ""         },    /* 	&#32;		Space  */
        { '!' ,   ""         },    /* 	&#33;		Exclamation mark  */
        { '"' ,   "&quot;"   },    /* 	Quotation mark  */
        { '#' ,   ""         },    /* 	&#35;		Number sign  */
        { '$' ,   ""         },    /* 	&#36;		Dollar sign  */
        { '%' ,   ""         },    /* 	&#37;		Percent sign  */
        { '&' ,   "&amp;"    },    /* 	Ampersand  */
        { '\'' ,  ""         },    /* 	&#39;		Apostrophe  */
        { '(' ,   ""         },    /* 	&#40;		Left parenthesis  */
        { ')' ,   ""         },    /* 	&#41;		Right parenthesis  */
        { '*' ,   ""         },    /* 	&#42;		Asterisk  */
        { '+' ,   ""         },    /* 	&#43;		Plus sign  */
        { ',' ,   ""         },    /* 	&#44;		Comma  */
        { '-' ,   ""         },    /* 	&#45;		Hyphen  */
        { '.' ,   ""         },    /* 	&#46;		Period (fullstop)  */
        { '/' ,   ""         },    /* 	&#47;		Solidus (slash)  */
        { '0' ,   ""         },    /* 	&#48;		Digit 0  */
        { '1' ,   ""         },    /* 	&#49;		Digit 1  */
        { '2' ,   ""         },    /* 	&#50;		Digit 2  */
        { '3' ,   ""         },    /* 	&#51;		Digit 3  */
        { '4' ,   ""         },    /* 	&#52;		Digit 4  */
        { '5' ,   ""         },    /* 	&#53;		Digit 5  */
        { '6' ,   ""         },    /* 	&#54;		Digit 6  */
        { '7' ,   ""         },    /* 	&#55;		Digit 7  */
        { '8' ,   ""         },    /* 	&#56;		Digit 8  */
        { '9' ,   ""         },    /* 	&#57;		Digit 9  */
        { ':' ,   ""         },    /* 	&#58;		Colon  */
        { ';' ,   ""         },    /* 	&#59;		Semicolon  */
        { '<' ,   "&lt;"     },    /* 	Less than  */
        { '=' ,   ""         },    /* 	&#61;		Equals sign  */
        { '>' ,   "&gt;"     },    /* 	Greater than  */
        { '?' ,   ""         },    /* 	&#63;		Question mark  */
        { '@' ,   ""         },    /* 	&#64;		Commercial at  */
        { 'A' ,   ""         },    /* 	&#65;		Capital A  */
        { 'B' ,   ""         },    /* 	&#66;		Capital B  */
        { 'C' ,   ""         },    /* 	&#67;		Capital C  */
        { 'D' ,   ""         },    /* 	&#68;		Capital D  */
        { 'E' ,   ""         },    /* 	&#69;		Capital E  */
        { 'F' ,   ""         },    /* 	&#70;		Capital F  */
        { 'G' ,   ""         },    /* 	&#71;		Capital G  */
        { 'H' ,   ""         },    /* 	&#72;		Capital H  */
        { 'I' ,   ""         },    /* 	&#73;		Capital I  */
        { 'J' ,   ""         },    /* 	&#74;		Capital J  */
        { 'K' ,   ""         },    /* 	&#75;		Capital K  */
        { 'L' ,   ""         },    /* 	&#76;		Capital L  */
        { 'M' ,   ""         },    /* 	&#77;		Capital M  */
        { 'N' ,   ""         },    /* 	&#78;		Capital N  */
        { 'O' ,   ""         },    /* 	&#79;		Capital O  */
        { 'P' ,   ""         },    /* 	&#80;		Capital P  */
        { 'Q' ,   ""         },    /* 	&#81;		Capital Q  */
        { 'R' ,   ""         },    /* 	&#82;		Capital R  */
        { 'S' ,   ""         },    /* 	&#83;		Capital S  */
        { 'T' ,   ""         },    /* 	&#84;		Capital T  */
        { 'U' ,   ""         },    /* 	&#85;		Capital U  */
        { 'V' ,   ""         },    /* 	&#86;		Capital V  */
        { 'W' ,   ""         },    /* 	&#87;		Capital W  */
        { 'X' ,   ""         },    /* 	&#88;		Capital X  */
        { 'Y' ,   ""         },    /* 	&#89;		Capital Y  */
        { 'Z' ,   ""         },    /* 	&#90;		Capital Z  */
        { '[' ,   ""         },    /* 	&#91;		Left square bracket  */
        { '\\' ,  ""         },    /* 	&#92;		Reverse solidus (backslash)  */
        { ']' ,   ""         },    /* 	&#93;		Right square bracket  */
        { '^' ,   ""         },    /* 	&#94;		Caret  */
        { '_' ,   ""         },    /* 	&#95;		Horizontal bar (underscore)  */
        { '`' ,   ""         },    /* 	&#96;		Acute accent  */
        { 'a' ,   ""         },    /* 	&#97;		Small a  */
        { 'b' ,   ""         },    /* 	&#98;		Small b  */
        { 'c' ,   ""         },    /* 	&#99;		Small c  */
        { 'd' ,   ""         },    /* 	&#100;		Small d  */
        { 'e' ,   ""         },    /* 	&#101;		Small e  */
        { 'f' ,   ""         },    /* 	&#102;		Small f  */
        { 'g' ,   ""         },    /* 	&#103;		Small g  */
        { 'h' ,   ""         },    /* 	&#104;		Small h  */
        { 'i' ,   ""         },    /* 	&#105;		Small i  */
        { 'j' ,   ""         },    /* 	&#106;		Small j  */
        { 'k' ,   ""         },    /* 	&#107;		Small k  */
        { 'l' ,   ""         },    /* 	&#108;		Small l  */
        { 'm' ,   ""         },    /* 	&#109;		Small m  */
        { 'n' ,   ""         },    /* 	&#110;		Small n  */
        { 'o' ,   ""         },    /* 	&#111;		Small o  */
        { 'p' ,   ""         },    /* 	&#112;		Small p  */
        { 'q' ,   ""         },    /* 	&#113;		Small q  */
        { 'r' ,   ""         },    /* 	&#114;		Small r  */
        { 's' ,   ""         },    /* 	&#115;		Small s  */
        { 't' ,   ""         },    /* 	&#116;		Small t  */
        { 'u' ,   ""         },    /* 	&#117;		Small u  */
        { 'v' ,   ""         },    /* 	&#118;		Small v  */
        { 'w' ,   ""         },    /* 	&#119;		Small w  */
        { 'x' ,   ""         },    /* 	&#120;		Small x  */
        { 'y' ,   ""         },    /* 	&#121;		Small y  */
        { 'z' ,   ""         },    /* 	&#122;		Small z  */
        { '{' ,   ""         },    /* 	&#123;		Left curly brace  */
        { '|' ,   ""         },    /* 	&#124;		Vertical bar  */
        { '}' ,   ""         },    /* 	&#125;		Right curly brace  */
        { '~' ,   ""         },    /* 	&#126;		Tilde  */
        { '' ,   ""         },    /* 	&#127;		Unused  */
        { '\x80' ,   "&#128;"         },    /* 	&#128;		Unused */
        { '\x81' ,   "&#129;"         },    /* &#129;		Unused  */
        { '\x82' ,   "&#130;"         },    /* &#130;		Unused  */
        { '\x83' ,   "&#131;"         },    /* &#131;		Unused  */
        { '\x84' ,   "&#132;"         },    /* &#132;		Unused  */
        { '\x85' ,   "&#133;"         },    /* &#133;		Unused  */
        { '\x86' ,   "&#134;"         },    /* &#134;		Unused  */
        { '\x87' ,   "&#135;"         },    /* &#135;		Unused  */
        { '\x88' ,   "&#136;"         },    /* &#136;		Unused  */
        { '\x89' ,   "&#137;"         },    /* &#137;		Unused  */
        { '\x8a' ,   "&#138;"         },    /* &#138;		Horizontal tab  */
        { '\x8b' ,   "&#139;"         },    /* &#139;		Line feed  */
        { '\x8c' ,   "&#140;"         },    /* &#140;		Unused  */
        { '\x8d' ,   "&#141;"         },    /* &#141;		Unused  */
        { '\x8e' ,   "&#142;"         },    /* &#142;		Carriage Return  */
        { '\x8f' ,   "&#143;"         },    /* &#143;		Unused  */
        { '\x90' ,   "&#144;"         },    /* &#144;		Unused  */
        { '\x91' ,   "&#145;"         },    /* &#145;		Unused  */
        { '\x92' ,   "&#146;"         },    /* &#146;		Unused  */
        { '\x93' ,   "&#147;"         },    /* &#147;		Unused  */
        { '\x94' ,   "&#148;"         },    /* &#148;		Unused  */
        { '\x95' ,   "&#149;"         },    /* &#149;		Unused  */
        { '\x96' ,   "&#150;"         },    /* &#150;		Unused  */
        { '\x97' ,   "&#151;"         },    /* &#151;		Unused  */
        { '\x98' ,   "&#152;"         },    /* &#152;		Unused  */
        { '\x99' ,   "&#153;"         },    /* &#153;		Unused  */
        { '\x9a' ,   "&#154;"         },    /* &#154;		Unused  */
        { '\x9b' ,   "&#155;"         },    /* &#155;		Unused  */
        { '\x9c' ,   "&#156;"         },    /* &#156;		Unused  */
        { '\x9d' ,   "&#157;"         },    /* &#157;		Unused  */
        { '\x9e' ,   "&#158;"         },    /* &#158;		Unused  */
        { '\x9f' ,   "&#159;"         },    /* &#159;		Unused  */
        { '\xa0' ,   "&nbsp;"   },    /* 	Non-breaking Space  */
        { '�' ,   "&iexcl;"  },    /* 	Inverted exclamation    */
        { '�' ,   "&cent;"   },    /* 	Cent sign               */
        { '�' ,   "&pound;"  },    /* 	Pound sterling  */
        { '�' ,   "&curren;" },    /* 	General currency sign  */
        { '�' ,   "&yen;"    },    /* 	Yen sign  */
/*        { '�' ,   "&brvbar;" },    / *  	Broken vertical bar  */
        { '�' ,   "&brkbar;" },    /* 	Broken vertical bar  */
        { '�' ,   "&sect;"  },    /* 	Section sign  */
/*        { '�' ,   "&&um;"    },    / *  	Di�resis / Umlaut  */
        { '�' ,   "&die;"   },    /* 	Di�resis / Umlaut  */
        { '�' ,   "&copy;"  },    /* 	Copyright               */
        { '�' ,   "&ordf;"  },    /* 	Feminine ordinal  */
        { '�' ,   "&laquo;" },    /* 	Left angle quote, guillemot left  */
        { '�' ,   "&euro;"   },    /*	Euro sign  */
        { '�' ,   "&shy;"    },    /* 	Soft hyphen  */
        { '�' ,   "&reg;"    },    /* 	Registered trademark  */
/*        { '�' ,   "&macr;"   },    / *  	Macron accent  */
        { '�' ,   "&hibar;"  },    /* 	Macron accent  */
        { '�' ,   "&deg;"    },    /* 	Degree sign  */
        { '�' ,   "&plusmn;" },    /* 	Plus or minus  */
        { '�' ,   "&sup2;"   },    /* 	Superscript two  */
        { '�' ,   "&sup3;"   },    /* 	Superscript three  */
        { '�' ,   "&acute;"  },    /* 	Acute accent  */
        { '�' ,   "&micro;"  },    /* 	Micro sign  */
        { '�' ,   "&para;"   },    /* 	Paragraph sign  */
        { '�' ,   "&middot;" },    /* 	Middle dot  */
        { '�' ,   "&cedil;"  },    /* 	Cedilla  */
        { '�' ,   "&sup1;"   },    /* 	Superscript one  */
        { '�' ,   "&ordm;"   },    /* 	Masculine ordinal  */
        { '�' ,   "&raquo;"  },    /* 	Right angle quote, guillemot right  */
        { '�' ,   "&frac14;" },    /* 	Fraction one-fourth  */
        { '�' ,   "&frac12;" },    /* 	Fraction one-half  */
        { '�' ,   "&frac34;" },    /* 	Fraction three-fourths  */
        { '�' ,   "&iquest;" },    /* 	Inverted question mark  */
        { '�' ,   "&Agrave;" },    /* 	Capital A, grave accent  */
        { '�' ,   "&Aacute;" },    /* 	Capital A, acute accent  */
        { '�' ,   "&Acirc;"  },    /* 	Capital A, circumflex  */
        { '�' ,   "&Atilde;" },    /* 	Capital A, tilde  */
        { '�' ,   "&Auml;"   },    /* 	Capital A, di�resis / umlaut  */
        { '�' ,   "&Aring;"  },    /* 	Capital A, ring  */
        { '�' ,   "&AElig;"  },    /* 	Capital AE ligature  */
        { '�' ,   "&Ccedil;" },    /* 	Capital C, cedilla  */
        { '�' ,   "&Egrave;" },    /* 	Capital E, grave accent  */
        { '�' ,   "&Eacute;" },    /* 	Capital E, acute accent  */
        { '�' ,   "&Ecirc;"  },    /* 	Capital E, circumflex  */
        { '�' ,   "&Euml;"   },    /* 	Capital E, di�resis / umlaut  */
        { '�' ,   "&Igrave;" },    /* 	Capital I, grave accent  */
        { '�' ,   "&Iacute;" },    /* 	Capital I, acute accent  */
        { '�' ,   "&Icirc;"  },    /* 	Capital I, circumflex  */
        { '�' ,   "&Iuml;"   },    /* 	Capital I, di�resis / umlaut  */
        { '�' ,   "&ETH;"    },    /* 	Capital Eth, Icelandic  */
        { '�' ,   "&Ntilde;" },    /* 	Capital N, tilde  */
        { '�' ,   "&Ograve;" },    /* 	Capital O, grave accent  */
        { '�' ,   "&Oacute;" },    /* 	Capital O, acute accent  */
        { '�' ,   "&Ocirc;"  },    /* 	Capital O, circumflex  */
        { '�' ,   "&Otilde;" },    /* 	Capital O, tilde  */
        { '�' ,   "&Ouml;"   },    /* 	Capital O, di�resis / umlaut  */
        { '�' ,   "&times;"  },    /* 	Multiply sign  */
        { '�' ,   "&Oslash;" },    /* 	Capital O, slash  */
        { '�' ,   "&Ugrave;" },    /* 	Capital U, grave accent  */
        { '�' ,   "&Uacute;" },    /* 	Capital U, acute accent  */
        { '�' ,   "&Ucirc;"  },    /* 	Capital U, circumflex  */
        { '�' ,   "&Uuml;"   },    /* 	Capital U, di�resis / umlaut  */
        { '�' ,   "&Yacute;" },    /* 	Capital Y, acute accent  */
        { '�' ,   "&THORN;"  },    /* 	Capital Thorn, Icelandic  */
        { '�' ,   "&szlig;"  },    /* 	Small sharp s, German sz  */
        { '�' ,   "&agrave;" },    /* 	Small a, grave accent  */
        { '�' ,   "&aacute;" },    /* 	Small a, acute accent  */
        { '�' ,   "&acirc;"  },    /* 	Small a, circumflex  */
        { '�' ,   "&atilde;" },    /* 	Small a, tilde  */
        { '�' ,   "&auml;"   },    /* 	Small a, di�resis / umlaut  */
        { '�' ,   "&aring;"  },    /* 	Small a, ring  */
        { '�' ,   "&aelig;"  },    /* 	Small ae ligature  */
        { '�' ,   "&ccedil;" },    /* 	Small c, cedilla  */
        { '�' ,   "&egrave;" },    /* 	Small e, grave accent  */
        { '�' ,   "&eacute;" },    /* 	Small e, acute accent  */
        { '�' ,   "&ecirc;"  },    /* 	Small e, circumflex  */
        { '�' ,   "&euml;"   },    /* 	Small e, di�resis / umlaut  */
        { '�' ,   "&igrave;" },    /* 	Small i, grave accent  */
        { '�' ,   "&iacute;" },    /* 	Small i, acute accent  */
        { '�' ,   "&icirc;"  },    /* 	Small i, circumflex  */
        { '�' ,   "&iuml;"   },    /* 	Small i, di�resis / umlaut  */
        { '�' ,   "&eth;"    },    /* 	Small eth, Icelandic  */
        { '�' ,   "&ntilde;" },    /* 	Small n, tilde  */
        { '�' ,   "&ograve;" },    /* 	Small o, grave accent  */
        { '�' ,   "&oacute;" },    /* 	Small o, acute accent  */
        { '�' ,   "&ocirc;"  },    /* 	Small o, circumflex  */
        { '�' ,   "&otilde;" },    /* 	Small o, tilde  */
        { '�' ,   "&ouml;"   },    /* 	Small o, di�resis / umlaut  */
        { '�' ,   "&divide;" },    /* 	Division sign  */
        { '�' ,   "&oslash;" },    /* 	Small o, slash  */
        { '�' ,   "&ugrave;" },    /* 	Small u, grave accent  */
        { '�' ,   "&uacute;" },    /* 	Small u, acute accent  */
        { '�' ,   "&ucirc;"  },    /* 	Small u, circumflex  */
        { '�' ,   "&uuml;"   },    /* 	Small u, di�resis / umlaut  */
        { '�' ,   "&yacute;" },    /* 	Small y, acute accent  */
        { '�' ,   "&thorn;"  },    /* 	Small thorn, Icelandic  */
        { '\255', "&yuml;"   },    /* 	Small y, di�resis / umlaut  */
    } ; 
 

struct tCharTrans Char2Url [] = 

    {
        { ' ' ,   "%00"         },    /* &#00;		Unused */ 
        { ' ' ,   "%01"         },    /* &#01;		Unused */
        { ' ' ,   "%02"         },    /* &#02;		Unused  */
        { ' ' ,   "%03"         },    /* &#03;		Unused  */
        { ' ' ,   "%04"         },    /* &#04;		Unused  */
        { ' ' ,   "%05"         },    /* &#05;		Unused  */
        { ' ' ,   "%06"         },    /* &#06;		Unused  */
        { ' ' ,   "%07"         },    /* &#07;		Unused  */
        { ' ' ,   "%08"         },    /* &#08;		Unused  */
        { ' ' ,   "%09"         },    /* &#09;		Horizontal tab  */
        { ' ' ,   "%0A"         },    /* &#10;		Line feed  */
        { ' ' ,   "%0B"         },    /* &#11;		Unused  */
        { ' ' ,   "%0C"         },    /* &#12;		Unused  */
        { ' ' ,   "%0D"         },    /* &#13;		Carriage Return  */
        { ' ' ,   "%0E"         },    /* &#14;		Unused  */
        { ' ' ,   "%0F"         },    /* &#15;		Unused  */
        { ' ' ,   "%10"         },    /* &#16;		Unused  */
        { ' ' ,   "%11"         },    /* &#17;		Unused  */
        { ' ' ,   "%12"         },    /* &#18;		Unused  */
        { ' ' ,   "%13"         },    /* &#19;		Unused  */
        { ' ' ,   "%14"         },    /* &#20;		Unused  */
        { ' ' ,   "%15"         },    /* &#21;		Unused  */
        { ' ' ,   "%16"         },    /* &#22;		Unused  */
        { ' ' ,   "%17"         },    /* &#23;		Unused  */
        { ' ' ,   "%18"         },    /* &#24;		Unused  */
        { ' ' ,   "%19"         },    /* &#25;		Unused  */
        { ' ' ,   "%1A"         },    /* &#26;		Unused  */
        { ' ' ,   "%1B"         },    /* &#27;		Unused  */
        { ' ' ,   "%1C"         },    /* &#28;		Unused  */
        { ' ' ,   "%1D"         },    /* &#29;		Unused  */
        { ' ' ,   "%1E"         },    /* &#30;		Unused  */
        { ' ' ,   "%1F"         },    /* &#31;		Unused  */
        { ' ' ,   "%20"           },    /* 	&#32;		Space  */
        { '!' ,   ""         },    /* 	&#33;		Exclamation mark  */
        { '"' ,   "%22"   },    /* 	Quotation mark  */
        { '#' ,   "%23"      },    /* 	&#35;		Number sign  */
        { '$' ,   ""         },    /* 	&#36;		Dollar sign  */
        { '%' ,   "%25"      },    /* 	&#37;		Percent sign  */
        { '&' ,   "%26"    },    /* 	Ampersand  */
        { '\'' ,  "%27"       },    /* 	&#39;		Apostrophe  */
        { '(' ,   ""         },    /* 	&#40;		Left parenthesis  */
        { ')' ,   ""         },    /* 	&#41;		Right parenthesis  */
        { '*' ,   ""         },    /* 	&#42;		Asterisk  */
        { '+' ,   "%2B"         },    /* 	&#43;		Plus sign  */
        { ',' ,   ""         },    /* 	&#44;		Comma  */
        { '-' ,   ""         },    /* 	&#45;		Hyphen  */
        { '.' ,   ""         },    /* 	&#46;		Period (fullstop)  */
        { '/' ,   ""         },    /* 	&#47;		Solidus (slash)  */
        { '0' ,   ""         },    /* 	&#48;		Digit 0  */
        { '1' ,   ""         },    /* 	&#49;		Digit 1  */
        { '2' ,   ""         },    /* 	&#50;		Digit 2  */
        { '3' ,   ""         },    /* 	&#51;		Digit 3  */
        { '4' ,   ""         },    /* 	&#52;		Digit 4  */
        { '5' ,   ""         },    /* 	&#53;		Digit 5  */
        { '6' ,   ""         },    /* 	&#54;		Digit 6  */
        { '7' ,   ""         },    /* 	&#55;		Digit 7  */
        { '8' ,   ""         },    /* 	&#56;		Digit 8  */
        { '9' ,   ""         },    /* 	&#57;		Digit 9  */
        { ':' ,   ""         },    /* 	&#58;		Colon  */
        { ';' ,   "%3B"      },    /* 	&#59;		Semicolon  */
        { '<' ,   "%3C"      },    /* 	Less than  */
        { '=' ,   "%3D"      },    /* 	&#61;		Equals sign  */
        { '>' ,   "%3E"      },    /* 	Greater than  */
        { '?' ,   "%3F"      },    /* 	&#63;		Question mark  */
        { '@' ,   ""      },    /* 	&#64;		Commercial at  */
        { 'A' ,   ""         },    /* 	&#65;		Capital A  */
        { 'B' ,   ""         },    /* 	&#66;		Capital B  */
        { 'C' ,   ""         },    /* 	&#67;		Capital C  */
        { 'D' ,   ""         },    /* 	&#68;		Capital D  */
        { 'E' ,   ""         },    /* 	&#69;		Capital E  */
        { 'F' ,   ""         },    /* 	&#70;		Capital F  */
        { 'G' ,   ""         },    /* 	&#71;		Capital G  */
        { 'H' ,   ""         },    /* 	&#72;		Capital H  */
        { 'I' ,   ""         },    /* 	&#73;		Capital I  */
        { 'J' ,   ""         },    /* 	&#74;		Capital J  */
        { 'K' ,   ""         },    /* 	&#75;		Capital K  */
        { 'L' ,   ""         },    /* 	&#76;		Capital L  */
        { 'M' ,   ""         },    /* 	&#77;		Capital M  */
        { 'N' ,   ""         },    /* 	&#78;		Capital N  */
        { 'O' ,   ""         },    /* 	&#79;		Capital O  */
        { 'P' ,   ""         },    /* 	&#80;		Capital P  */
        { 'Q' ,   ""         },    /* 	&#81;		Capital Q  */
        { 'R' ,   ""         },    /* 	&#82;		Capital R  */
        { 'S' ,   ""         },    /* 	&#83;		Capital S  */
        { 'T' ,   ""         },    /* 	&#84;		Capital T  */
        { 'U' ,   ""         },    /* 	&#85;		Capital U  */
        { 'V' ,   ""         },    /* 	&#86;		Capital V  */
        { 'W' ,   ""         },    /* 	&#87;		Capital W  */
        { 'X' ,   ""         },    /* 	&#88;		Capital X  */
        { 'Y' ,   ""         },    /* 	&#89;		Capital Y  */
        { 'Z' ,   ""         },    /* 	&#90;		Capital Z  */
        { '[' ,   ""         },    /* 	&#91;		Left square bracket  */
        { '\\' ,  ""         },    /* 	&#92;		Reverse solidus (backslash)  */
        { ']' ,   ""         },    /* 	&#93;		Right square bracket  */
        { '^' ,   ""         },    /* 	&#94;		Caret  */
        { '_' ,   ""         },    /* 	&#95;		Horizontal bar (underscore)  */
        { '`' ,   ""         },    /* 	&#96;		Acute accent  */
        { 'a' ,   ""         },    /* 	&#97;		Small a  */
        { 'b' ,   ""         },    /* 	&#98;		Small b  */
        { 'c' ,   ""         },    /* 	&#99;		Small c  */
        { 'd' ,   ""         },    /* 	&#100;		Small d  */
        { 'e' ,   ""         },    /* 	&#101;		Small e  */
        { 'f' ,   ""         },    /* 	&#102;		Small f  */
        { 'g' ,   ""         },    /* 	&#103;		Small g  */
        { 'h' ,   ""         },    /* 	&#104;		Small h  */
        { 'i' ,   ""         },    /* 	&#105;		Small i  */
        { 'j' ,   ""         },    /* 	&#106;		Small j  */
        { 'k' ,   ""         },    /* 	&#107;		Small k  */
        { 'l' ,   ""         },    /* 	&#108;		Small l  */
        { 'm' ,   ""         },    /* 	&#109;		Small m  */
        { 'n' ,   ""         },    /* 	&#110;		Small n  */
        { 'o' ,   ""         },    /* 	&#111;		Small o  */
        { 'p' ,   ""         },    /* 	&#112;		Small p  */
        { 'q' ,   ""         },    /* 	&#113;		Small q  */
        { 'r' ,   ""         },    /* 	&#114;		Small r  */
        { 's' ,   ""         },    /* 	&#115;		Small s  */
        { 't' ,   ""         },    /* 	&#116;		Small t  */
        { 'u' ,   ""         },    /* 	&#117;		Small u  */
        { 'v' ,   ""         },    /* 	&#118;		Small v  */
        { 'w' ,   ""         },    /* 	&#119;		Small w  */
        { 'x' ,   ""         },    /* 	&#120;		Small x  */
        { 'y' ,   ""         },    /* 	&#121;		Small y  */
        { 'z' ,   ""         },    /* 	&#122;		Small z  */
        { '{' ,   ""         },    /* 	&#123;		Left curly brace  */
        { '|' ,   ""         },    /* 	&#124;		Vertical bar  */
        { '}' ,   ""         },    /* 	&#125;		Right curly brace  */
        { '~' ,   ""         },    /* 	&#126;		Tilde  */
        { '' ,   ""         },    /* 	&#127;		Unused  */
        { '�' ,   ""         },    /* 	&#128;		Unused */
         
        { ' ' ,   ""         },    /* &#129;		Unused  */
        { ' ' ,   ""         },    /* &#130;		Unused  */
        { ' ' ,   ""         },    /* &#131;		Unused  */
        { ' ' ,   ""         },    /* &#132;		Unused  */
        { ' ' ,   ""         },    /* &#133;		Unused  */
        { ' ' ,   ""         },    /* &#134;		Unused  */
        { ' ' ,   ""         },    /* &#135;		Unused  */
        { ' ' ,   ""         },    /* &#136;		Unused  */
        { ' ' ,   ""         },    /* &#137;		Unused  */
        { ' ' ,   ""         },    /* &#138;		Horizontal tab  */
        { ' ' ,   ""         },    /* &#139;		Line feed  */
        { ' ' ,   ""         },    /* &#140;		Unused  */
        { ' ' ,   ""         },    /* &#141;		Unused  */
        { ' ' ,   ""         },    /* &#142;		Carriage Return  */
        { ' ' ,   ""         },    /* &#143;		Unused  */
        { ' ' ,   ""         },    /* &#144;		Unused  */
        { ' ' ,   ""         },    /* &#145;		Unused  */
        { ' ' ,   ""         },    /* &#146;		Unused  */
        { ' ' ,   ""         },    /* &#147;		Unused  */
        { ' ' ,   ""         },    /* &#148;		Unused  */
        { ' ' ,   ""         },    /* &#149;		Unused  */
        { ' ' ,   ""         },    /* &#150;		Unused  */
        { ' ' ,   ""         },    /* &#151;		Unused  */
        { ' ' ,   ""         },    /* &#152;		Unused  */
        { ' ' ,   ""         },    /* &#153;		Unused  */
        { ' ' ,   ""         },    /* &#154;		Unused  */
        { ' ' ,   ""         },    /* &#155;		Unused  */
        { ' ' ,   ""         },    /* &#156;		Unused  */
        { ' ' ,   ""         },    /* &#157;		Unused  */
        { ' ' ,   ""         },    /* &#158;		Unused  */
        { ' ' ,   ""         },    /* &#159;		Unused  */
        { '�' ,   ""   },    /* 	Non-breaking Space  */
        { '�' ,   ""  },    /* 	Inverted exclamation    */
        { '�' ,   ""   },    /* 	Cent sign               */
        { '�' ,   ""  },    /* 	Pound sterling  */
        { '�' ,   "" },    /* 	General currency sign  */
        { '�' ,   ""    },    /* 	Yen sign  */
        { '�' ,   "" },    /* 	Broken vertical bar  */
        { '�' ,   ""  },    /* 	Section sign  */
        { '�' ,   ""   },    /* 	Di�resis / Umlaut  */
        { '�' ,   ""  },    /* 	Copyright               */
        { '�' ,   ""  },    /* 	Feminine ordinal  */
        { '�' ,   "" },    /* 	Left angle quote, guillemot left  */
        { '�' ,   ""   },    /*	Not sign  */
        { '�' ,   ""    },    /* 	Soft hyphen  */
        { '�' ,   ""    },    /* 	Registered trademark  */
        { '�' ,   ""  },    /* 	Macron accent  */
        { '�' ,   ""    },    /* 	Degree sign  */
        { '�' ,   "" },    /* 	Plus or minus  */
        { '�' ,   ""   },    /* 	Superscript two  */
        { '�' ,   ""   },    /* 	Superscript three  */
        { '�' ,   ""  },    /* 	Acute accent  */
        { '�' ,   ""  },    /* 	Micro sign  */
        { '�' ,   ""   },    /* 	Paragraph sign  */
        { '�' ,   "" },    /* 	Middle dot  */
        { '�' ,   ""  },    /* 	Cedilla  */
        { '�' ,   ""   },    /* 	Superscript one  */
        { '�' ,   ""   },    /* 	Masculine ordinal  */
        { '�' ,   ""  },    /* 	Right angle quote, guillemot right  */
        { '�' ,   "" },    /* 	Fraction one-fourth  */
        { '�' ,   "" },    /* 	Fraction one-half  */
        { '�' ,   "" },    /* 	Fraction three-fourths  */
        { '�' ,   "" },    /* 	Inverted question mark  */
        { '�' ,   "" },    /* 	Capital A, grave accent  */
        { '�' ,   "" },    /* 	Capital A, acute accent  */
        { '�' ,   ""  },    /* 	Capital A, circumflex  */
        { '�' ,   "" },    /* 	Capital A, tilde  */
        { '�' ,   ""   },    /* 	Capital A, di�resis / umlaut  */
        { '�' ,   ""  },    /* 	Capital A, ring  */
        { '�' ,   ""  },    /* 	Capital AE ligature  */
        { '�' ,   "" },    /* 	Capital C, cedilla  */
        { '�' ,   "" },    /* 	Capital E, grave accent  */
        { '�' ,   "" },    /* 	Capital E, acute accent  */
        { '�' ,   ""  },    /* 	Capital E, circumflex  */
        { '�' ,   ""   },    /* 	Capital E, di�resis / umlaut  */
        { '�' ,   "" },    /* 	Capital I, grave accent  */
        { '�' ,   "" },    /* 	Capital I, acute accent  */
        { '�' ,   ""  },    /* 	Capital I, circumflex  */
        { '�' ,   ""   },    /* 	Capital I, di�resis / umlaut  */
        { '�' ,   ""    },    /* 	Capital Eth, Icelandic  */
        { '�' ,   "" },    /* 	Capital N, tilde  */
        { '�' ,   "" },    /* 	Capital O, grave accent  */
        { '�' ,   "" },    /* 	Capital O, acute accent  */
        { '�' ,   ""  },    /* 	Capital O, circumflex  */
        { '�' ,   "" },    /* 	Capital O, tilde  */
        { '�' ,   ""   },    /* 	Capital O, di�resis / umlaut  */
        { '�' ,   ""  },    /* 	Multiply sign  */
        { '�' ,   "" },    /* 	Capital O, slash  */
        { '�' ,   "" },    /* 	Capital U, grave accent  */
        { '�' ,   "" },    /* 	Capital U, acute accent  */
        { '�' ,   ""  },    /* 	Capital U, circumflex  */
        { '�' ,   ""   },    /* 	Capital U, di�resis / umlaut  */
        { '�' ,   "" },    /* 	Capital Y, acute accent  */
        { '�' ,   ""  },    /* 	Capital Thorn, Icelandic  */
        { '�' ,   ""  },    /* 	Small sharp s, German sz  */
        { '�' ,   "" },    /* 	Small a, grave accent  */
        { '�' ,   "" },    /* 	Small a, acute accent  */
        { '�' ,   ""  },    /* 	Small a, circumflex  */
        { '�' ,   "" },    /* 	Small a, tilde  */
        { '�' ,   ""   },    /* 	Small a, di�resis / umlaut  */
        { '�' ,   ""  },    /* 	Small a, ring  */
        { '�' ,   ""  },    /* 	Small ae ligature  */
        { '�' ,   "" },    /* 	Small c, cedilla  */
        { '�' ,   "" },    /* 	Small e, grave accent  */
        { '�' ,   "" },    /* 	Small e, acute accent  */
        { '�' ,   ""  },    /* 	Small e, circumflex  */
        { '�' ,   ""   },    /* 	Small e, di�resis / umlaut  */
        { '�' ,   "" },    /* 	Small i, grave accent  */
        { '�' ,   "" },    /* 	Small i, acute accent  */
        { '�' ,   ""  },    /* 	Small i, circumflex  */
        { '�' ,   ""   },    /* 	Small i, di�resis / umlaut  */
        { '�' ,   ""    },    /* 	Small eth, Icelandic  */
        { '�' ,   "" },    /* 	Small n, tilde  */
        { '�' ,   "" },    /* 	Small o, grave accent  */
        { '�' ,   "" },    /* 	Small o, acute accent  */
        { '�' ,   ""  },    /* 	Small o, circumflex  */
        { '�' ,   "" },    /* 	Small o, tilde  */
        { '�' ,   ""   },    /* 	Small o, di�resis / umlaut  */
        { '�' ,   "" },    /* 	Division sign  */
        { '�' ,   "" },    /* 	Small o, slash  */
        { '�' ,   "" },    /* 	Small u, grave accent  */
        { '�' ,   "" },    /* 	Small u, acute accent  */
        { '�' ,   ""  },    /* 	Small u, circumflex  */
        { '�' ,   ""   },    /* 	Small u, di�resis / umlaut  */
        { '�' ,   "" },    /* 	Small y, acute accent  */
        { '�' ,   ""  },    /* 	Small thorn, Icelandic  */
        { '\255', ""   },    /* 	Small y, di�resis / umlaut  */
    } ; 
    
struct tCharTrans Char2XML [] = 

    {
        { ' ' ,   ""         },    /* &#00;		Unused */ 
        { ' ' ,   ""         },    /* &#01;		Unused */
        { ' ' ,   ""         },    /* &#02;		Unused  */
        { ' ' ,   ""         },    /* &#03;		Unused  */
        { ' ' ,   ""         },    /* &#04;		Unused  */
        { ' ' ,   ""         },    /* &#05;		Unused  */
        { ' ' ,   ""         },    /* &#06;		Unused  */
        { ' ' ,   ""         },    /* &#07;		Unused  */
        { ' ' ,   ""         },    /* &#08;		Unused  */
        { ' ' ,   ""         },    /* &#09;		Horizontal tab  */
        { ' ' ,   ""         },    /* &#10;		Line feed  */
        { ' ' ,   ""         },    /* &#11;		Unused  */
        { ' ' ,   ""         },    /* &#12;		Unused  */
        { ' ' ,   ""         },    /* &#13;		Carriage Return  */
        { ' ' ,   ""         },    /* &#14;		Unused  */
        { ' ' ,   ""         },    /* &#15;		Unused  */
        { ' ' ,   ""         },    /* &#16;		Unused  */
        { ' ' ,   ""         },    /* &#17;		Unused  */
        { ' ' ,   ""         },    /* &#18;		Unused  */
        { ' ' ,   ""         },    /* &#19;		Unused  */
        { ' ' ,   ""         },    /* &#20;		Unused  */
        { ' ' ,   ""         },    /* &#21;		Unused  */
        { ' ' ,   ""         },    /* &#22;		Unused  */
        { ' ' ,   ""         },    /* &#23;		Unused  */
        { ' ' ,   ""         },    /* &#24;		Unused  */
        { ' ' ,   ""         },    /* &#25;		Unused  */
        { ' ' ,   ""         },    /* &#26;		Unused  */
        { ' ' ,   ""         },    /* &#27;		Unused  */
        { ' ' ,   ""         },    /* &#28;		Unused  */
        { ' ' ,   ""         },    /* &#29;		Unused  */
        { ' ' ,   ""         },    /* &#30;		Unused  */
        { ' ' ,   ""         },    /* &#31;		Unused  */
        { ' ' ,   ""         },    /* 	&#32;		Space  */
        { '!' ,   ""         },    /* 	&#33;		Exclamation mark  */
        { '"' ,   "&quot;"   },    /* 	Quotation mark  */
        { '#' ,   ""         },    /* 	&#35;		Number sign  */
        { '$' ,   ""         },    /* 	&#36;		Dollar sign  */
        { '%' ,   ""         },    /* 	&#37;		Percent sign  */
        { '&' ,   "&amp;"    },    /* 	Ampersand  */
        { '\'' ,  "&apos;"         },    /* 	&#39;		Apostrophe  */
        { '(' ,   ""         },    /* 	&#40;		Left parenthesis  */
        { ')' ,   ""         },    /* 	&#41;		Right parenthesis  */
        { '*' ,   ""         },    /* 	&#42;		Asterisk  */
        { '+' ,   ""         },    /* 	&#43;		Plus sign  */
        { ',' ,   ""         },    /* 	&#44;		Comma  */
        { '-' ,   ""         },    /* 	&#45;		Hyphen  */
        { '.' ,   ""         },    /* 	&#46;		Period (fullstop)  */
        { '/' ,   ""         },    /* 	&#47;		Solidus (slash)  */
        { '0' ,   ""         },    /* 	&#48;		Digit 0  */
        { '1' ,   ""         },    /* 	&#49;		Digit 1  */
        { '2' ,   ""         },    /* 	&#50;		Digit 2  */
        { '3' ,   ""         },    /* 	&#51;		Digit 3  */
        { '4' ,   ""         },    /* 	&#52;		Digit 4  */
        { '5' ,   ""         },    /* 	&#53;		Digit 5  */
        { '6' ,   ""         },    /* 	&#54;		Digit 6  */
        { '7' ,   ""         },    /* 	&#55;		Digit 7  */
        { '8' ,   ""         },    /* 	&#56;		Digit 8  */
        { '9' ,   ""         },    /* 	&#57;		Digit 9  */
        { ':' ,   ""         },    /* 	&#58;		Colon  */
        { ';' ,   ""         },    /* 	&#59;		Semicolon  */
        { '<' ,   "&lt;"     },    /* 	Less than  */
        { '=' ,   ""         },    /* 	&#61;		Equals sign  */
        { '>' ,   "&gt;"     },    /* 	Greater than  */
        { '?' ,   ""         },    /* 	&#63;		Question mark  */
        { '@' ,   ""         },    /* 	&#64;		Commercial at  */
        { 'A' ,   ""         },    /* 	&#65;		Capital A  */
        { 'B' ,   ""         },    /* 	&#66;		Capital B  */
        { 'C' ,   ""         },    /* 	&#67;		Capital C  */
        { 'D' ,   ""         },    /* 	&#68;		Capital D  */
        { 'E' ,   ""         },    /* 	&#69;		Capital E  */
        { 'F' ,   ""         },    /* 	&#70;		Capital F  */
        { 'G' ,   ""         },    /* 	&#71;		Capital G  */
        { 'H' ,   ""         },    /* 	&#72;		Capital H  */
        { 'I' ,   ""         },    /* 	&#73;		Capital I  */
        { 'J' ,   ""         },    /* 	&#74;		Capital J  */
        { 'K' ,   ""         },    /* 	&#75;		Capital K  */
        { 'L' ,   ""         },    /* 	&#76;		Capital L  */
        { 'M' ,   ""         },    /* 	&#77;		Capital M  */
        { 'N' ,   ""         },    /* 	&#78;		Capital N  */
        { 'O' ,   ""         },    /* 	&#79;		Capital O  */
        { 'P' ,   ""         },    /* 	&#80;		Capital P  */
        { 'Q' ,   ""         },    /* 	&#81;		Capital Q  */
        { 'R' ,   ""         },    /* 	&#82;		Capital R  */
        { 'S' ,   ""         },    /* 	&#83;		Capital S  */
        { 'T' ,   ""         },    /* 	&#84;		Capital T  */
        { 'U' ,   ""         },    /* 	&#85;		Capital U  */
        { 'V' ,   ""         },    /* 	&#86;		Capital V  */
        { 'W' ,   ""         },    /* 	&#87;		Capital W  */
        { 'X' ,   ""         },    /* 	&#88;		Capital X  */
        { 'Y' ,   ""         },    /* 	&#89;		Capital Y  */
        { 'Z' ,   ""         },    /* 	&#90;		Capital Z  */
        { '[' ,   ""         },    /* 	&#91;		Left square bracket  */
        { '\\' ,  ""         },    /* 	&#92;		Reverse solidus (backslash)  */
        { ']' ,   ""         },    /* 	&#93;		Right square bracket  */
        { '^' ,   ""         },    /* 	&#94;		Caret  */
        { '_' ,   ""         },    /* 	&#95;		Horizontal bar (underscore)  */
        { '`' ,   ""         },    /* 	&#96;		Acute accent  */
        { 'a' ,   ""         },    /* 	&#97;		Small a  */
        { 'b' ,   ""         },    /* 	&#98;		Small b  */
        { 'c' ,   ""         },    /* 	&#99;		Small c  */
        { 'd' ,   ""         },    /* 	&#100;		Small d  */
        { 'e' ,   ""         },    /* 	&#101;		Small e  */
        { 'f' ,   ""         },    /* 	&#102;		Small f  */
        { 'g' ,   ""         },    /* 	&#103;		Small g  */
        { 'h' ,   ""         },    /* 	&#104;		Small h  */
        { 'i' ,   ""         },    /* 	&#105;		Small i  */
        { 'j' ,   ""         },    /* 	&#106;		Small j  */
        { 'k' ,   ""         },    /* 	&#107;		Small k  */
        { 'l' ,   ""         },    /* 	&#108;		Small l  */
        { 'm' ,   ""         },    /* 	&#109;		Small m  */
        { 'n' ,   ""         },    /* 	&#110;		Small n  */
        { 'o' ,   ""         },    /* 	&#111;		Small o  */
        { 'p' ,   ""         },    /* 	&#112;		Small p  */
        { 'q' ,   ""         },    /* 	&#113;		Small q  */
        { 'r' ,   ""         },    /* 	&#114;		Small r  */
        { 's' ,   ""         },    /* 	&#115;		Small s  */
        { 't' ,   ""         },    /* 	&#116;		Small t  */
        { 'u' ,   ""         },    /* 	&#117;		Small u  */
        { 'v' ,   ""         },    /* 	&#118;		Small v  */
        { 'w' ,   ""         },    /* 	&#119;		Small w  */
        { 'x' ,   ""         },    /* 	&#120;		Small x  */
        { 'y' ,   ""         },    /* 	&#121;		Small y  */
        { 'z' ,   ""         },    /* 	&#122;		Small z  */
        { '{' ,   ""         },    /* 	&#123;		Left curly brace  */
        { '|' ,   ""         },    /* 	&#124;		Vertical bar  */
        { '}' ,   ""         },    /* 	&#125;		Right curly brace  */
        { '~' ,   ""         },    /* 	&#126;		Tilde  */
        { '' ,   ""         },    /* 	&#127;		Unused  */
        { '�' ,   ""         },    /* 	&#128;		Unused */
        { ' ' ,   ""         },    /* &#129;		Unused  */
        { ' ' ,   ""         },    /* &#130;		Unused  */
        { ' ' ,   ""         },    /* &#131;		Unused  */
        { ' ' ,   ""         },    /* &#132;		Unused  */
        { ' ' ,   ""         },    /* &#133;		Unused  */
        { ' ' ,   ""         },    /* &#134;		Unused  */
        { ' ' ,   ""         },    /* &#135;		Unused  */
        { ' ' ,   ""         },    /* &#136;		Unused  */
        { ' ' ,   ""         },    /* &#137;		Unused  */
        { ' ' ,   ""         },    /* &#138;		Horizontal tab  */
        { ' ' ,   ""         },    /* &#139;		Line feed  */
        { ' ' ,   ""         },    /* &#140;		Unused  */
        { ' ' ,   ""         },    /* &#141;		Unused  */
        { ' ' ,   ""         },    /* &#142;		Carriage Return  */
        { ' ' ,   ""         },    /* &#143;		Unused  */
        { ' ' ,   ""         },    /* &#144;		Unused  */
        { ' ' ,   ""         },    /* &#145;		Unused  */
        { ' ' ,   ""         },    /* &#146;		Unused  */
        { ' ' ,   ""         },    /* &#147;		Unused  */
        { ' ' ,   ""         },    /* &#148;		Unused  */
        { ' ' ,   ""         },    /* &#149;		Unused  */
        { ' ' ,   ""         },    /* &#150;		Unused  */
        { ' ' ,   ""         },    /* &#151;		Unused  */
        { ' ' ,   ""         },    /* &#152;		Unused  */
        { ' ' ,   ""         },    /* &#153;		Unused  */
        { ' ' ,   ""         },    /* &#154;		Unused  */
        { ' ' ,   ""         },    /* &#155;		Unused  */
        { ' ' ,   ""         },    /* &#156;		Unused  */
        { ' ' ,   ""         },    /* &#157;		Unused  */
        { ' ' ,   ""         },    /* &#158;		Unused  */
        { ' ' ,   ""         },    /* &#159;		Unused  */
        { ' ' ,   ""   },    /* 	Non-breaking Space  */
        { '�' ,   ""  },    /* 	Inverted exclamation    */
        { '�' ,   ""   },    /* 	Cent sign               */
        { '�' ,   ""  },    /* 	Pound sterling  */
        { '�' ,   "" },    /* 	General currency sign  */
        { '�' ,   ""    },    /* 	Yen sign  */
/*        { '�' ,   "" },    / *  	Broken vertical bar  */
        { '�' ,   "" },    /* 	Broken vertical bar  */
        { '�' ,   ""  },    /* 	Section sign  */
/*        { '�' ,   ""    },    / *  	Di�resis / Umlaut  */
        { '�' ,   ""   },    /* 	Di�resis / Umlaut  */
        { '�' ,   ""  },    /* 	Copyright               */
        { '�' ,   ""  },    /* 	Feminine ordinal  */
        { '�' ,   "" },    /* 	Left angle quote, guillemot left  */
        { '�' ,   ""   },    /*	Not sign  */
        { '�' ,   ""    },    /* 	Soft hyphen  */
        { '�' ,   ""    },    /* 	Registered trademark  */
/*        { '�' ,   ""   },    / *  	Macron accent  */
        { '�' ,   ""  },    /* 	Macron accent  */
        { '�' ,   ""    },    /* 	Degree sign  */
        { '�' ,   "" },    /* 	Plus or minus  */
        { '�' ,   ""   },    /* 	Superscript two  */
        { '�' ,   ""   },    /* 	Superscript three  */
        { '�' ,   ""  },    /* 	Acute accent  */
        { '�' ,   ""  },    /* 	Micro sign  */
        { '�' ,   ""   },    /* 	Paragraph sign  */
        { '�' ,   "" },    /* 	Middle dot  */
        { '�' ,   ""  },    /* 	Cedilla  */
        { '�' ,   ""   },    /* 	Superscript one  */
        { '�' ,   ""   },    /* 	Masculine ordinal  */
        { '�' ,   ""  },    /* 	Right angle quote, guillemot right  */
        { '�' ,   "" },    /* 	Fraction one-fourth  */
        { '�' ,   "" },    /* 	Fraction one-half  */
        { '�' ,   "" },    /* 	Fraction three-fourths  */
        { '�' ,   "" },    /* 	Inverted question mark  */
        { '�' ,   "" },    /* 	Capital A, grave accent  */
        { '�' ,   "" },    /* 	Capital A, acute accent  */
        { '�' ,   ""  },    /* 	Capital A, circumflex  */
        { '�' ,   "" },    /* 	Capital A, tilde  */
        { '�' ,   ""   },    /* 	Capital A, di�resis / umlaut  */
        { '�' ,   ""  },    /* 	Capital A, ring  */
        { '�' ,   ""  },    /* 	Capital AE ligature  */
        { '�' ,   "" },    /* 	Capital C, cedilla  */
        { '�' ,   "" },    /* 	Capital E, grave accent  */
        { '�' ,   "" },    /* 	Capital E, acute accent  */
        { '�' ,   ""  },    /* 	Capital E, circumflex  */
        { '�' ,   ""   },    /* 	Capital E, di�resis / umlaut  */
        { '�' ,   "" },    /* 	Capital I, grave accent  */
        { '�' ,   "" },    /* 	Capital I, acute accent  */
        { '�' ,   ""  },    /* 	Capital I, circumflex  */
        { '�' ,   ""   },    /* 	Capital I, di�resis / umlaut  */
        { '�' ,   ""    },    /* 	Capital Eth, Icelandic  */
        { '�' ,   "" },    /* 	Capital N, tilde  */
        { '�' ,   "" },    /* 	Capital O, grave accent  */
        { '�' ,   "" },    /* 	Capital O, acute accent  */
        { '�' ,   ""  },    /* 	Capital O, circumflex  */
        { '�' ,   "" },    /* 	Capital O, tilde  */
        { '�' ,   ""   },    /* 	Capital O, di�resis / umlaut  */
        { '�' ,   ""  },    /* 	Multiply sign  */
        { '�' ,   "" },    /* 	Capital O, slash  */
        { '�' ,   "" },    /* 	Capital U, grave accent  */
        { '�' ,   "" },    /* 	Capital U, acute accent  */
        { '�' ,   ""  },    /* 	Capital U, circumflex  */
        { '�' ,   ""   },    /* 	Capital U, di�resis / umlaut  */
        { '�' ,   "" },    /* 	Capital Y, acute accent  */
        { '�' ,   ""  },    /* 	Capital Thorn, Icelandic  */
        { '�' ,   ""  },    /* 	Small sharp s, German sz  */
        { '�' ,   "" },    /* 	Small a, grave accent  */
        { '�' ,   "" },    /* 	Small a, acute accent  */
        { '�' ,   ""  },    /* 	Small a, circumflex  */
        { '�' ,   "" },    /* 	Small a, tilde  */
        { '�' ,   ""   },    /* 	Small a, di�resis / umlaut  */
        { '�' ,   ""  },    /* 	Small a, ring  */
        { '�' ,   ""  },    /* 	Small ae ligature  */
        { '�' ,   "" },    /* 	Small c, cedilla  */
        { '�' ,   "" },    /* 	Small e, grave accent  */
        { '�' ,   "" },    /* 	Small e, acute accent  */
        { '�' ,   ""  },    /* 	Small e, circumflex  */
        { '�' ,   ""   },    /* 	Small e, di�resis / umlaut  */
        { '�' ,   "" },    /* 	Small i, grave accent  */
        { '�' ,   "" },    /* 	Small i, acute accent  */
        { '�' ,   ""  },    /* 	Small i, circumflex  */
        { '�' ,   ""   },    /* 	Small i, di�resis / umlaut  */
        { '�' ,   ""    },    /* 	Small eth, Icelandic  */
        { '�' ,   "" },    /* 	Small n, tilde  */
        { '�' ,   "" },    /* 	Small o, grave accent  */
        { '�' ,   "" },    /* 	Small o, acute accent  */
        { '�' ,   ""  },    /* 	Small o, circumflex  */
        { '�' ,   "" },    /* 	Small o, tilde  */
        { '�' ,   ""   },    /* 	Small o, di�resis / umlaut  */
        { '�' ,   "" },    /* 	Division sign  */
        { '�' ,   "" },    /* 	Small o, slash  */
        { '�' ,   "" },    /* 	Small u, grave accent  */
        { '�' ,   "" },    /* 	Small u, acute accent  */
        { '�' ,   ""  },    /* 	Small u, circumflex  */
        { '�' ,   ""   },    /* 	Small u, di�resis / umlaut  */
        { '�' ,   "" },    /* 	Small y, acute accent  */
        { '�' ,   ""  },    /* 	Small thorn, Icelandic  */
        { '\255', ""   },    /* 	Small y, di�resis / umlaut  */
    } ; 
 
    
struct tCharTrans Html2Char [] = 
 
    {
        { '\x80' ,   "&#128;"         },    /* &#129;		Unused  */
        { '\x81' ,   "&#129;"         },    /* &#129;		Unused  */
        { '\x82' ,   "&#130;"         },    /* &#130;		Unused  */
        { '\x83' ,   "&#131;"         },    /* &#131;		Unused  */
        { '\x84' ,   "&#132;"         },    /* &#132;		Unused  */
        { '\x85' ,   "&#133;"         },    /* &#133;		Unused  */
        { '\x86' ,   "&#134;"         },    /* &#134;		Unused  */
        { '\x87' ,   "&#135;"         },    /* &#135;		Unused  */
        { '\x88' ,   "&#136;"         },    /* &#136;		Unused  */
        { '\x89' ,   "&#137;"         },    /* &#137;		Unused  */
        { '\x8a' ,   "&#138;"         },    /* &#138;		Horizontal tab  */
        { '\x8b' ,   "&#139;"         },    /* &#139;		Line feed  */
        { '\x8c' ,   "&#140;"         },    /* &#140;		Unused  */
        { '\x8d' ,   "&#141;"         },    /* &#141;		Unused  */
        { '\x8e' ,   "&#142;"         },    /* &#142;		Carriage Return  */
        { '\x8f' ,   "&#143;"         },    /* &#143;		Unused  */
        { '\x90' ,   "&#144;"         },    /* &#144;		Unused  */
        { '\x91' ,   "&#145;"         },    /* &#145;		Unused  */
        { '\x92' ,   "&#146;"         },    /* &#146;		Unused  */
        { '\x93' ,   "&#147;"         },    /* &#147;		Unused  */
        { '\x94' ,   "&#148;"         },    /* &#148;		Unused  */
        { '\x95' ,   "&#149;"         },    /* &#149;		Unused  */
        { '\x96' ,   "&#150;"         },    /* &#150;		Unused  */
        { '\x97' ,   "&#151;"         },    /* &#151;		Unused  */
        { '\x98' ,   "&#152;"         },    /* &#152;		Unused  */
        { '\x99' ,   "&#153;"         },    /* &#153;		Unused  */
        { '\x9a' ,   "&#154;"         },    /* &#154;		Unused  */
        { '\x9b' ,   "&#155;"         },    /* &#155;		Unused  */
        { '\x9c' ,   "&#156;"         },    /* &#156;		Unused  */
        { '\x9d' ,   "&#157;"         },    /* &#157;		Unused  */
        { '\x9e' ,   "&#158;"         },    /* &#158;		Unused  */
        { '\x9f' ,   "&#159;"         },    /* &#159;		Unused  */
        { '�' ,   "&AElig"  },    /* 	Capital AE ligature  */
        { '�' ,   "&Aacute" },    /* 	Capital A, acute accent  */
        { '�' ,   "&Acirc"  },    /* 	Capital A, circumflex  */
        { '�' ,   "&Agrave" },    /* 	Capital A, grave accent  */
        { '�' ,   "&Aring"  },    /* 	Capital A, ring  */
        { '�' ,   "&Atilde" },    /* 	Capital A, tilde  */
        { '�' ,   "&Auml"   },    /* 	Capital A, di�resis / umlaut  */
        { '�' ,   "&Ccedil" },    /* 	Capital C, cedilla  */
        { '�' ,   "&ETH"    },    /* 	Capital Eth, Icelandic  */
        { '�' ,   "&Eacute" },    /* 	Capital E, acute accent  */
        { '�' ,   "&Ecirc"  },    /* 	Capital E, circumflex  */
        { '�' ,   "&Egrave" },    /* 	Capital E, grave accent  */
        { '�' ,   "&Euml"   },    /* 	Capital E, di�resis / umlaut  */
        { '�' ,   "&Iacute" },    /* 	Capital I, acute accent  */
        { '�' ,   "&Icirc"  },    /* 	Capital I, circumflex  */
        { '�' ,   "&Igrave" },    /* 	Capital I, grave accent  */
        { '�' ,   "&Iuml"   },    /* 	Capital I, di�resis / umlaut  */
        { '�' ,   "&Ntilde" },    /* 	Capital N, tilde  */
        { '�' ,   "&Oacute" },    /* 	Capital O, acute accent  */
        { '�' ,   "&Ocirc"  },    /* 	Capital O, circumflex  */
        { '�' ,   "&Ograve" },    /* 	Capital O, grave accent  */
        { '�' ,   "&Oslash" },    /* 	Capital O, slash  */
        { '�' ,   "&Otilde" },    /* 	Capital O, tilde  */
        { '�' ,   "&Ouml"   },    /* 	Capital O, di�resis / umlaut  */
        { '�' ,   "&THORN"  },    /* 	Capital Thorn, Icelandic  */
        { '�' ,   "&Uacute" },    /* 	Capital U, acute accent  */
        { '�' ,   "&Ucirc"  },    /* 	Capital U, circumflex  */
        { '�' ,   "&Ugrave" },    /* 	Capital U, grave accent  */
        { '�' ,   "&Uuml"   },    /* 	Capital U, di�resis / umlaut  */
        { '�' ,   "&Yacute" },    /* 	Capital Y, acute accent  */
        { '�' ,   "&aacute" },    /* 	Small a, acute accent  */
        { '�' ,   "&acirc"  },    /* 	Small a, circumflex  */
        { '�' ,   "&acute"  },    /* 	Acute accent  */
        { '�' ,   "&aelig"  },    /* 	Small ae ligature  */
        { '�' ,   "&agrave" },    /* 	Small a, grave accent  */
        { '&' ,   "&amp"    },    /* 	Ampersand  */
        { '�' ,   "&aring"  },    /* 	Small a, ring  */
        { '�' ,   "&atilde" },    /* 	Small a, tilde  */
        { '�' ,   "&auml"   },    /* 	Small a, di�resis / umlaut  */
        { '�' ,   "&brkbar" },    /* 	Broken vertical bar  */
        { '�' ,   "&brvbar" },    /*  	Broken vertical bar  */
        { '�' ,   "&ccedil" },    /* 	Small c, cedilla  */
        { '�' ,   "&cedil"  },    /* 	Cedilla  */
        { '�' ,   "&cent"   },    /* 	Cent sign  */
        { '�' ,   "&copy"  },    /* 	Copyright  */
        { '�' ,   "&curren" },    /* 	General currency sign  */
        { '�' ,   "&deg"    },    /* 	Degree sign  */
        { '�' ,   "&die"   },    /* 	Di�resis / Umlaut  */
        { '�' ,   "&divide" },    /* 	Division sign  */
        { '�' ,   "&eacute" },    /* 	Small e, acute accent  */
        { '�' ,   "&ecirc"  },    /* 	Small e, circumflex  */
        { '�' ,   "&egrave" },    /* 	Small e, grave accent  */
        { '�' ,   "&eth"    },    /* 	Small eth, Icelandic  */
        { '�' ,   "&euml"   },    /* 	Small e, di�resis / umlaut  */
        { '�' ,   "&euro"   },    /*	Euro sign  */
        { '�' ,   "&frac12" },    /* 	Fraction one-half  */
        { '�' ,   "&frac14" },    /* 	Fraction one-fourth  */
        { '�' ,   "&frac34" },    /* 	Fraction three-fourths  */
        { '>' ,   "&gt"     },    /* 	Greater than  */
        { '�' ,   "&hibar"  },    /* 	Macron accent  */
        { '�' ,   "&iacute" },    /* 	Small i, acute accent  */
        { '�' ,   "&icirc"  },    /* 	Small i, circumflex  */
        { '�' ,   "&iexcl"  },    /* 	Inverted exclamation  */
        { '�' ,   "&igrave" },    /* 	Small i, grave accent  */
        { '�' ,   "&iquest" },    /* 	Inverted question mark  */
        { '�' ,   "&iuml"   },    /* 	Small i, di�resis / umlaut  */
        { '�' ,   "&laquo" },    /* 	Left angle quote, guillemot left  */
        { '<' ,   "&lt"     },    /* 	Less than  */
        { '�' ,   "&macr"   },    /*  	Macron accent  */
        { '�' ,   "&micro"  },    /* 	Micro sign  */
        { '�' ,   "&middot" },    /* 	Middle dot  */
        { ' ' ,   "&nbsp"   },    /* 	Non-breaking Space  */
        { '�' ,   "&ntilde" },    /* 	Small n, tilde  */
        { '�' ,   "&oacute" },    /* 	Small o, acute accent  */
        { '�' ,   "&ocirc"  },    /* 	Small o, circumflex  */
        { '�' ,   "&ograve" },    /* 	Small o, grave accent  */
        { '�' ,   "&ordf"  },    /* 	Feminine ordinal  */
        { '�' ,   "&ordm"   },    /* 	Masculine ordinal  */
        { '�' ,   "&oslash" },    /* 	Small o, slash  */
        { '�' ,   "&otilde" },    /* 	Small o, tilde  */
        { '�' ,   "&ouml"   },    /* 	Small o, di�resis / umlaut  */
        { '�' ,   "&para"   },    /* 	Paragraph sign  */
        { '�' ,   "&plusmn" },    /* 	Plus or minus  */
        { '�' ,   "&pound"  },    /* 	Pound sterling  */
        { '"' ,   "&quot"   },    /* 	Quotation mark  */
        { '�' ,   "&raquo"  },    /* 	Right angle quote, guillemot right  */
        { '�' ,   "&reg"    },    /* 	Registered trademark  */
        { '�' ,   "&sect"  },    /* 	Section sign  */
        { '�' ,   "&shy"    },    /* 	Soft hyphen  */
        { '�' ,   "&sup1"   },    /* 	Superscript one  */
        { '�' ,   "&sup2"   },    /* 	Superscript two  */
        { '�' ,   "&sup3"   },    /* 	Superscript three  */
        { '�' ,   "&szlig"  },    /* 	Small sharp s, German sz  */
        { '�' ,   "&thorn"  },    /* 	Small thorn, Icelandic  */
        { '�' ,   "&times"  },    /* 	Multiply sign  */
        { '�' ,   "&uacute" },    /* 	Small u, acute accent  */
        { '�' ,   "&ucirc"  },    /* 	Small u, circumflex  */
        { '�' ,   "&ugrave" },    /* 	Small u, grave accent  */
        { '�' ,   "&um"    },    /*  	Di�resis / Umlaut  */
        { '�' ,   "&uuml"   },    /* 	Small u, di�resis / umlaut  */
        { '�' ,   "&yacute" },    /* 	Small y, acute accent  */
        { '�' ,   "&yen"    },    /* 	Yen sign  */
        { '\255', "&yuml"   },    /* 	Small y, di�resis / umlaut  */
} ;


int sizeHtml2Char = sizeof (Html2Char) / sizeof (struct tCharTrans) ;

