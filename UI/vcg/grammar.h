

#line 1 "/RW/esprit/users/sander/src/PARSEGEN/help.skel"
 
/*--------------------------------------------------------------------*/
/*  Scanner and Parser Interface 			              */
/*--------------------------------------------------------------------*/

/* $Id$ */

#ifndef SCANPARSE_H
#define SCANPARSE_H
 
extern int line_nr;
extern int pos_nr;

#ifdef ANSI_C
void init_lex(void);
#else
void init_lex();
#endif

#ifndef yysyntaxtree
#define yysyntaxtree char*
#endif



#line 32000 "/RW/esprit/users/sander/src/PARSEGEN/parse.skel"
typedef struct stree_node *syntaxtree;
#undef yysyntaxtree
#define yysyntaxtree syntaxtree


#line 1 "/RW/esprit/users/sander/src/PARSEGEN/stdph.skel"

#ifndef STDPARSER
#define STDPARSER

/* $Id$ */

#undef  PARSEGENSTD
#define PARSEGENSTD

/*--------------------------------------------------------------------*/
/*  Standard Tree Construction Interface   			      */
/*--------------------------------------------------------------------*/

#ifndef ALIGN
#define ALIGN 8
#define IALIGN (ALIGN-1)
#endif
#ifndef PARSEBLOCKSIZE
#define PARSEBLOCKSIZE 10000
#endif

/*-------------------*/
/* syntax tree nodes */
/*-------------------*/

union  special {
        unsigned char      byte;
        short int          snum;
        unsigned short int usnum;
        int                num;
        unsigned int       unum;
        long int           lnum;
        unsigned long int  ulnum;
        float              realnum;
        double             lrealnum;
        char              *string;
};

struct stree_node {
        int  tag_field;
        int  first_line;
        int  first_column;
        int  last_line;
        int  last_column;
#ifdef USERFTYPE
	USERFTYPE user_field;
#endif
        struct stree_node *father;
        union  special     contents;
        struct stree_node *xson[1];
};


/* typedef struct stree_node *syntaxtree; */


#undef yysyntaxtree
#define yysyntaxtree syntaxtree 


#define tag(x)           ((x)->tag_field)
#define nr_of_sons(x)    (ConstructorArity((x)->tag_field))
#define xfirst_line(x)    ((x)->first_line)
#define xfirst_column(x)  ((x)->first_column)
#define xlast_line(x)     ((x)->last_line)
#define xlast_column(x)   ((x)->last_column)
#define xfather(x)        ((x)->father)

#ifdef USERFTYPE
#define	user_field(x)     ((x)->user_field)
#endif

#define get_byte(x)      ((x)->contents.byte)
#define get_snum(x)      ((x)->contents.snum)
#define get_usnum(x)     ((x)->contents.usnum)
#define get_num(x)       ((x)->contents.num)
#define get_unum(x)      ((x)->contents.unum)
#define get_lnum(x)      ((x)->contents.lnum)
#define get_ulnum(x)     ((x)->contents.ulnum)
#define get_realnum(x)   ((x)->contents.realnum)
#define get_lrealnum(x)  ((x)->contents.lrealnum)
#define get_string(x)    ((x)->contents.string)

#define son1(x)    ((x)->xson[0])
#define son2(x)    ((x)->xson[1])
#define son3(x)    ((x)->xson[2])
#define son4(x)    ((x)->xson[3])
#define son5(x)    ((x)->xson[4])
#define son6(x)    ((x)->xson[5])
#define son7(x)    ((x)->xson[6])
#define son8(x)    ((x)->xson[7])
#define son9(x)    ((x)->xson[8])
#define son(x,i)   ((x)->xson[i-1])

#ifndef Y_TAB_H


#line 1441 "grammar.pgs"
#include "y.tab.h"


#line 97 "/RW/esprit/users/sander/src/PARSEGEN/stdph.skel"
#define Y_TAB_H
#endif /* Y_TAB_H */


/*------------*/ 
/* Prototypes */ 
/*------------*/ 


#ifdef ANSI_C

char * ParseMalloc(int x);
void ParseFree(void);

union special *UnionByte(unsigned char x);
union special *UnionSnum(short int x);
union special *UnionUsnum(unsigned short int x);
union special *UnionNum(int x);
union special *UnionUnum(unsigned int x);
union special *UnionLnum(long int x);
union special *UnionUlnum(unsigned long int x);
union special *UnionRealnum(float x);
union special *UnionLrealnum(double x);
union special *UnionString(char *x);

syntaxtree BuildCont(int mtag,union special *x,YYLTYPE *l);
yysyntaxtree BuildTree(int mtag,int len,union special *x,YYLTYPE *l, ...);

syntaxtree Copy(syntaxtree x);
syntaxtree Revert(syntaxtree list);

const char *ConstructorName(int i);
int   ConstructorArity(int i);

#else
char * ParseMalloc();
void ParseFree();

union special *UnionByte();                     
union special *UnionSnum();                     
union special *UnionUsnum();                     
union special *UnionNum();                     
union special *UnionUnum();                     
union special *UnionLnum();                     
union special *UnionUlnum();                     
union special *UnionRealnum();                     
union special *UnionLrealnum();                     
union special *UnionString();                     

syntaxtree BuildCont();
yysyntaxtree BuildTree();

syntaxtree Copy();
syntaxtree Revert();

char *ConstructorName();
int   ConstructorArity();

#ifndef const
#define const
#endif

#endif /* ANSI_C */
 
#undef  yyparseinit
#define yyparseinit() /**/ 

#endif /* STDPARSER */

/*-- end of standard tree construction interface ---------------------*/



#line 1441 "grammar.pgs"

/* Constructors: */

#define T_Co_index_val  0
#define T_Co_stern  1
#define T_Co_range  2
#define T_Co_index  3
#define T_Co_index_value  4
#define T_Co_string  5
#define T_Co_char  6
#define T_Co_float  7
#define T_Co_integer  8
#define T_Co_z  9
#define T_Co_y  10
#define T_Co_x  11
#define T_Co_right_neighbor  12
#define T_Co_left_neighbor  13
#define T_Co_lower_neighbor  14
#define T_Co_upper_neighbor  15
#define T_Co_right_margin  16
#define T_Co_left_margin  17
#define T_Co_bottom_margin  18
#define T_Co_top_margin  19
#define T_Co_equal_column  20
#define T_Co_equal_row  21
#define T_Co_equal_position  22
#define T_Co_behind  23
#define T_Co_in_font  24
#define T_Co_right  25
#define T_Co_left  26
#define T_Co_below  27
#define T_Co_above  28
#define T_Co_limit  29
#define T_Co_cluster  30
#define T_Co_xrange  31
#define T_Co_high_margin  32
#define T_Co_low_margin  33
#define T_Co_neighbors  34
#define T_Co_greater  35
#define T_Co_smaller  36
#define T_Co_equal  37
#define T_Co_string_array  38
#define T_Co_dimension  39
#define T_Co_name  40
#define T_Co_interval  41
#define T_Co_nodes  42
#define T_Co_size  43
#define T_Co_solid  44
#define T_Co_line  45
#define T_Co_none  46
#define T_Co_invisible  47
#define T_Co_dashed  48
#define T_Co_dotted  49
#define T_Co_continuous  50
#define T_Co_anchor  51
#define T_Co_linestyle  52
#define T_Co_barrowstyle  53
#define T_Co_arrowstyle  54
#define T_Co_barrowsize  55
#define T_Co_arrowsize  56
#define T_Co_barrowcolor  57
#define T_Co_arrowcolor  58
#define T_Co_arrowheight  59
#define T_Co_arrowwidth  60
#define T_Co_priority  61
#define T_Co_class  62
#define T_Co_thickness  63
#define T_Co_targetname  64
#define T_Co_sourcename  65
#define T_Co_around  66
#define T_Co_top  67
#define T_Co_bottom  68
#define T_Co_triangle  69
#define T_Co_ellipse  70
#define T_Co_rhomb  71
#define T_Co_box  72
#define T_Co_right_justify  73
#define T_Co_left_justify  74
#define T_Co_center  75
#define T_Co_iconstyle  76
#define T_Co_iconheight  77
#define T_Co_iconwidth  78
#define T_Co_anchorpoints  79
#define T_Co_iconfile  80
#define T_Co_bordercolor  81
#define T_Co_fontname  82
#define T_Co_constraint_attribute  83
#define T_Co_edge_attribute  84
#define T_Co_node_attribute  85
#define T_Co_free  86
#define T_Co_fixed  87
#define T_Co_fpfish  88
#define T_Co_pfish  89
#define T_Co_fcfish  90
#define T_Co_cfish  91
#define T_Co_medianbary  92
#define T_Co_barymedian  93
#define T_Co_median  94
#define T_Co_bary  95
#define T_Co_no  96
#define T_Co_yes  97
#define T_Co_grey  98
#define T_Co_manual  99
#define T_Co_every  100
#define T_Co_depthfirst  101
#define T_Co_minbackwards  102
#define T_Co_minoutdegree  103
#define T_Co_maxoutdegree  104
#define T_Co_minindegree  105
#define T_Co_maxindegree  106
#define T_Co_mindegree  107
#define T_Co_maxdegree  108
#define T_Co_mindepthslow  109
#define T_Co_maxdepthslow  110
#define T_Co_mindepth  111
#define T_Co_maxdepth  112
#define T_Co_tree  113
#define T_Co_constaints  114
#define T_Co_planar  115
#define T_Co_isi  116
#define T_Co_barycenter  117
#define T_Co_right_to_left  118
#define T_Co_left_to_right  119
#define T_Co_bottom_to_top  120
#define T_Co_top_to_bottom  121
#define T_Co_low  122
#define T_Co_high  123
#define T_Co_colindex  124
#define T_Co_yellowgreen  125
#define T_Co_yellow  126
#define T_Co_white  127
#define T_Co_turquoise  128
#define T_Co_red  129
#define T_Co_purple  130
#define T_Co_pink  131
#define T_Co_orchid  132
#define T_Co_orange  133
#define T_Co_magenta  134
#define T_Co_lilac  135
#define T_Co_lightyellow  136
#define T_Co_lightred  137
#define T_Co_lightmagenta  138
#define T_Co_lightgrey  139
#define T_Co_lightgreen  140
#define T_Co_lightcyan  141
#define T_Co_lightblue  142
#define T_Co_khaki  143
#define T_Co_green  144
#define T_Co_gold  145
#define T_Co_darkyellow  146
#define T_Co_darkred  147
#define T_Co_darkmagenta  148
#define T_Co_darkgrey  149
#define T_Co_darkgreen  150
#define T_Co_darkcyan  151
#define T_Co_darkblue  152
#define T_Co_cyan  153
#define T_Co_blue  154
#define T_Co_black  155
#define T_Co_aquamarine  156
#define T_Co_yscrollbar  157
#define T_Co_xscrollbar  158
#define T_Co_outputfunction  159
#define T_Co_inputfunction  160
#define T_Co_topsort  161
#define T_Co_layoutparameter  162
#define T_Co_include  163
#define T_Co_typename  164
#define T_Co_straight_max  165
#define T_Co_rubber_min  166
#define T_Co_rubber_max  167
#define T_Co_pendel_min  168
#define T_Co_pendel_max  169
#define T_Co_cross_min  170
#define T_Co_cross_max  171
#define T_Co_bend_max  172
#define T_Co_view_splines  173
#define T_Co_view_nodes  174
#define T_Co_view_edges  175
#define T_Co_view_method  176
#define T_Co_crossing_weight  177
#define T_Co_crossing_opt  178
#define T_Co_crossing_phase2  179
#define T_Co_treefactor  180
#define T_Co_spreadlevel  181
#define T_Co_arrow_mode  182
#define T_Co_port_sharing  183
#define T_Co_node_alignment  184
#define T_Co_orientation  185
#define T_Co_dummy  186
#define T_Co_nonearedges  187
#define T_Co_smanhatten  188
#define T_Co_manhatten  189
#define T_Co_priophase  190
#define T_Co_straightphase  191
#define T_Co_hidesingles  192
#define T_Co_finetuning  193
#define T_Co_dirty_edge_label  194
#define T_Co_display_edge_label  195
#define T_Co_late_edge_label  196
#define T_Co_splinefactor  197
#define T_Co_nearfactor  198
#define T_Co_upfactor  199
#define T_Co_downfactor  200
#define T_Co_layoutfrequency  201
#define T_Co_layoutalgorithm  202
#define T_Co_colentry  203
#define T_Co_infoname  204
#define T_Co_classname  205
#define T_Co_hidden  206
#define T_Co_yraster  207
#define T_Co_xlraster  208
#define T_Co_xraster  209
#define T_Co_yspace  210
#define T_Co_xlspace  211
#define T_Co_xspace  212
#define T_Co_ybase  213
#define T_Co_xbase  214
#define T_Co_ymax  215
#define T_Co_xmax  216
#define T_Co_status  217
#define T_Co_horizontal_order  218
#define T_Co_level  219
#define T_Co_shape  220
#define T_Co_textmode  221
#define T_Co_stretch  222
#define T_Co_shrink  223
#define T_Co_scaling  224
#define T_Co_folding  225
#define T_Co_loc  226
#define T_Co_ydef  227
#define T_Co_xdef  228
#define T_Co_borderwidth  229
#define T_Co_height  230
#define T_Co_width  231
#define T_Co_colorborder  232
#define T_Co_textcolor  233
#define T_Co_color  234
#define T_Co_info3  235
#define T_Co_info2  236
#define T_Co_info1  237
#define T_Co_label  238
#define T_Co_title  239
#define T_Co_constraint  240
#define T_Co_back_edge  241
#define T_Co_bent_near_edge  242
#define T_Co_near_edge  243
#define T_Co_edge  244
#define T_Co_node  245
#define T_Co_graph  246
#define T_Co_foldedge_defaults  247
#define T_Co_foldnode_defaults  248
#define T_Co_edge_defaults  249
#define T_Co_node_defaults  250
#define T_Co_graph_attribute  251
#define T_Co_graph_entry  252

/* Build Macros */

#define T_index_val(s0,l) BuildCont(T_Co_index_val,UnionLnum(s0),l)
#define T_stern(s0,l) BuildTree(T_Co_stern,1,UnionNum(0),l,s0)
#define T_range(s0,s1,l) BuildTree(T_Co_range,2,UnionNum(0),l,s0,s1)
#define T_index(s0,s1,l) BuildTree(T_Co_index,2,UnionNum(0),l,s0,s1)
#define T_index_value(s0,s1,l) BuildTree(T_Co_index_value,2,UnionNum(0),l,s0,s1)
#define T_string(s0,l) BuildCont(T_Co_string,UnionLnum(s0),l)
#define T_char(s0,l) BuildCont(T_Co_char,UnionByte(s0),l)
#define T_float(s0,l) BuildCont(T_Co_float,UnionLrealnum(s0),l)
#define T_integer(s0,l) BuildCont(T_Co_integer,UnionLnum(s0),l)
#define T_z(l) BuildCont(T_Co_z,UnionNum(0),l)
#define T_y(l) BuildCont(T_Co_y,UnionNum(0),l)
#define T_x(l) BuildCont(T_Co_x,UnionNum(0),l)
#define T_right_neighbor(l) BuildCont(T_Co_right_neighbor,UnionNum(0),l)
#define T_left_neighbor(l) BuildCont(T_Co_left_neighbor,UnionNum(0),l)
#define T_lower_neighbor(l) BuildCont(T_Co_lower_neighbor,UnionNum(0),l)
#define T_upper_neighbor(l) BuildCont(T_Co_upper_neighbor,UnionNum(0),l)
#define T_right_margin(l) BuildCont(T_Co_right_margin,UnionNum(0),l)
#define T_left_margin(l) BuildCont(T_Co_left_margin,UnionNum(0),l)
#define T_bottom_margin(l) BuildCont(T_Co_bottom_margin,UnionNum(0),l)
#define T_top_margin(l) BuildCont(T_Co_top_margin,UnionNum(0),l)
#define T_equal_column(l) BuildCont(T_Co_equal_column,UnionNum(0),l)
#define T_equal_row(l) BuildCont(T_Co_equal_row,UnionNum(0),l)
#define T_equal_position(l) BuildCont(T_Co_equal_position,UnionNum(0),l)
#define T_behind(l) BuildCont(T_Co_behind,UnionNum(0),l)
#define T_in_font(l) BuildCont(T_Co_in_font,UnionNum(0),l)
#define T_right(l) BuildCont(T_Co_right,UnionNum(0),l)
#define T_left(l) BuildCont(T_Co_left,UnionNum(0),l)
#define T_below(l) BuildCont(T_Co_below,UnionNum(0),l)
#define T_above(l) BuildCont(T_Co_above,UnionNum(0),l)
#define T_limit(l) BuildCont(T_Co_limit,UnionNum(0),l)
#define T_cluster(l) BuildCont(T_Co_cluster,UnionNum(0),l)
#define T_xrange(l) BuildCont(T_Co_xrange,UnionNum(0),l)
#define T_high_margin(l) BuildCont(T_Co_high_margin,UnionNum(0),l)
#define T_low_margin(l) BuildCont(T_Co_low_margin,UnionNum(0),l)
#define T_neighbors(l) BuildCont(T_Co_neighbors,UnionNum(0),l)
#define T_greater(l) BuildCont(T_Co_greater,UnionNum(0),l)
#define T_smaller(l) BuildCont(T_Co_smaller,UnionNum(0),l)
#define T_equal(l) BuildCont(T_Co_equal,UnionNum(0),l)
#define T_string_array(s0,s1,l) BuildTree(T_Co_string_array,2,UnionNum(0),l,s0,s1)
#define T_dimension(s0,l) BuildTree(T_Co_dimension,1,UnionNum(0),l,s0)
#define T_name(s0,l) BuildTree(T_Co_name,1,UnionNum(0),l,s0)
#define T_interval(s0,l) BuildTree(T_Co_interval,1,UnionNum(0),l,s0)
#define T_nodes(s0,l) BuildTree(T_Co_nodes,1,UnionNum(0),l,s0)
#define T_size(s0,l) BuildTree(T_Co_size,1,UnionNum(0),l,s0)
#define T_solid(l) BuildCont(T_Co_solid,UnionNum(0),l)
#define T_line(l) BuildCont(T_Co_line,UnionNum(0),l)
#define T_none(l) BuildCont(T_Co_none,UnionNum(0),l)
#define T_invisible(l) BuildCont(T_Co_invisible,UnionNum(0),l)
#define T_dashed(l) BuildCont(T_Co_dashed,UnionNum(0),l)
#define T_dotted(l) BuildCont(T_Co_dotted,UnionNum(0),l)
#define T_continuous(l) BuildCont(T_Co_continuous,UnionNum(0),l)
#define T_anchor(s0,l) BuildTree(T_Co_anchor,1,UnionNum(0),l,s0)
#define T_linestyle(s0,l) BuildTree(T_Co_linestyle,1,UnionNum(0),l,s0)
#define T_barrowstyle(s0,l) BuildTree(T_Co_barrowstyle,1,UnionNum(0),l,s0)
#define T_arrowstyle(s0,l) BuildTree(T_Co_arrowstyle,1,UnionNum(0),l,s0)
#define T_barrowsize(s0,l) BuildTree(T_Co_barrowsize,1,UnionNum(0),l,s0)
#define T_arrowsize(s0,l) BuildTree(T_Co_arrowsize,1,UnionNum(0),l,s0)
#define T_barrowcolor(s0,l) BuildTree(T_Co_barrowcolor,1,UnionNum(0),l,s0)
#define T_arrowcolor(s0,l) BuildTree(T_Co_arrowcolor,1,UnionNum(0),l,s0)
#define T_arrowheight(s0,l) BuildTree(T_Co_arrowheight,1,UnionNum(0),l,s0)
#define T_arrowwidth(s0,l) BuildTree(T_Co_arrowwidth,1,UnionNum(0),l,s0)
#define T_priority(s0,l) BuildTree(T_Co_priority,1,UnionNum(0),l,s0)
#define T_class(s0,l) BuildTree(T_Co_class,1,UnionNum(0),l,s0)
#define T_thickness(s0,l) BuildTree(T_Co_thickness,1,UnionNum(0),l,s0)
#define T_targetname(s0,l) BuildTree(T_Co_targetname,1,UnionNum(0),l,s0)
#define T_sourcename(s0,l) BuildTree(T_Co_sourcename,1,UnionNum(0),l,s0)
#define T_around(l) BuildCont(T_Co_around,UnionNum(0),l)
#define T_top(l) BuildCont(T_Co_top,UnionNum(0),l)
#define T_bottom(l) BuildCont(T_Co_bottom,UnionNum(0),l)
#define T_triangle(l) BuildCont(T_Co_triangle,UnionNum(0),l)
#define T_ellipse(l) BuildCont(T_Co_ellipse,UnionNum(0),l)
#define T_rhomb(l) BuildCont(T_Co_rhomb,UnionNum(0),l)
#define T_box(l) BuildCont(T_Co_box,UnionNum(0),l)
#define T_right_justify(l) BuildCont(T_Co_right_justify,UnionNum(0),l)
#define T_left_justify(l) BuildCont(T_Co_left_justify,UnionNum(0),l)
#define T_center(l) BuildCont(T_Co_center,UnionNum(0),l)
#define T_iconstyle(s0,l) BuildTree(T_Co_iconstyle,1,UnionNum(0),l,s0)
#define T_iconheight(s0,l) BuildTree(T_Co_iconheight,1,UnionNum(0),l,s0)
#define T_iconwidth(s0,l) BuildTree(T_Co_iconwidth,1,UnionNum(0),l,s0)
#define T_anchorpoints(s0,l) BuildTree(T_Co_anchorpoints,1,UnionNum(0),l,s0)
#define T_iconfile(s0,l) BuildTree(T_Co_iconfile,1,UnionNum(0),l,s0)
#define T_bordercolor(s0,l) BuildTree(T_Co_bordercolor,1,UnionNum(0),l,s0)
#define T_fontname(s0,l) BuildTree(T_Co_fontname,1,UnionNum(0),l,s0)
#define T_constraint_attribute(s0,s1,l) BuildTree(T_Co_constraint_attribute,2,UnionNum(0),l,s0,s1)
#define T_edge_attribute(s0,s1,l) BuildTree(T_Co_edge_attribute,2,UnionNum(0),l,s0,s1)
#define T_node_attribute(s0,s1,l) BuildTree(T_Co_node_attribute,2,UnionNum(0),l,s0,s1)
#define T_free(l) BuildCont(T_Co_free,UnionNum(0),l)
#define T_fixed(l) BuildCont(T_Co_fixed,UnionNum(0),l)
#define T_fpfish(l) BuildCont(T_Co_fpfish,UnionNum(0),l)
#define T_pfish(l) BuildCont(T_Co_pfish,UnionNum(0),l)
#define T_fcfish(l) BuildCont(T_Co_fcfish,UnionNum(0),l)
#define T_cfish(l) BuildCont(T_Co_cfish,UnionNum(0),l)
#define T_medianbary(l) BuildCont(T_Co_medianbary,UnionNum(0),l)
#define T_barymedian(l) BuildCont(T_Co_barymedian,UnionNum(0),l)
#define T_median(l) BuildCont(T_Co_median,UnionNum(0),l)
#define T_bary(l) BuildCont(T_Co_bary,UnionNum(0),l)
#define T_no(l) BuildCont(T_Co_no,UnionNum(0),l)
#define T_yes(l) BuildCont(T_Co_yes,UnionNum(0),l)
#define T_grey(l) BuildCont(T_Co_grey,UnionNum(0),l)
#define T_manual(l) BuildCont(T_Co_manual,UnionNum(0),l)
#define T_every(l) BuildCont(T_Co_every,UnionNum(0),l)
#define T_depthfirst(l) BuildCont(T_Co_depthfirst,UnionNum(0),l)
#define T_minbackwards(l) BuildCont(T_Co_minbackwards,UnionNum(0),l)
#define T_minoutdegree(l) BuildCont(T_Co_minoutdegree,UnionNum(0),l)
#define T_maxoutdegree(l) BuildCont(T_Co_maxoutdegree,UnionNum(0),l)
#define T_minindegree(l) BuildCont(T_Co_minindegree,UnionNum(0),l)
#define T_maxindegree(l) BuildCont(T_Co_maxindegree,UnionNum(0),l)
#define T_mindegree(l) BuildCont(T_Co_mindegree,UnionNum(0),l)
#define T_maxdegree(l) BuildCont(T_Co_maxdegree,UnionNum(0),l)
#define T_mindepthslow(l) BuildCont(T_Co_mindepthslow,UnionNum(0),l)
#define T_maxdepthslow(l) BuildCont(T_Co_maxdepthslow,UnionNum(0),l)
#define T_mindepth(l) BuildCont(T_Co_mindepth,UnionNum(0),l)
#define T_maxdepth(l) BuildCont(T_Co_maxdepth,UnionNum(0),l)
#define T_tree(l) BuildCont(T_Co_tree,UnionNum(0),l)
#define T_constaints(l) BuildCont(T_Co_constaints,UnionNum(0),l)
#define T_planar(l) BuildCont(T_Co_planar,UnionNum(0),l)
#define T_isi(l) BuildCont(T_Co_isi,UnionNum(0),l)
#define T_barycenter(l) BuildCont(T_Co_barycenter,UnionNum(0),l)
#define T_right_to_left(l) BuildCont(T_Co_right_to_left,UnionNum(0),l)
#define T_left_to_right(l) BuildCont(T_Co_left_to_right,UnionNum(0),l)
#define T_bottom_to_top(l) BuildCont(T_Co_bottom_to_top,UnionNum(0),l)
#define T_top_to_bottom(l) BuildCont(T_Co_top_to_bottom,UnionNum(0),l)
#define T_low(l) BuildCont(T_Co_low,UnionNum(0),l)
#define T_high(l) BuildCont(T_Co_high,UnionNum(0),l)
#define T_colindex(s0,l) BuildTree(T_Co_colindex,1,UnionNum(0),l,s0)
#define T_yellowgreen(l) BuildCont(T_Co_yellowgreen,UnionNum(0),l)
#define T_yellow(l) BuildCont(T_Co_yellow,UnionNum(0),l)
#define T_white(l) BuildCont(T_Co_white,UnionNum(0),l)
#define T_turquoise(l) BuildCont(T_Co_turquoise,UnionNum(0),l)
#define T_red(l) BuildCont(T_Co_red,UnionNum(0),l)
#define T_purple(l) BuildCont(T_Co_purple,UnionNum(0),l)
#define T_pink(l) BuildCont(T_Co_pink,UnionNum(0),l)
#define T_orchid(l) BuildCont(T_Co_orchid,UnionNum(0),l)
#define T_orange(l) BuildCont(T_Co_orange,UnionNum(0),l)
#define T_magenta(l) BuildCont(T_Co_magenta,UnionNum(0),l)
#define T_lilac(l) BuildCont(T_Co_lilac,UnionNum(0),l)
#define T_lightyellow(l) BuildCont(T_Co_lightyellow,UnionNum(0),l)
#define T_lightred(l) BuildCont(T_Co_lightred,UnionNum(0),l)
#define T_lightmagenta(l) BuildCont(T_Co_lightmagenta,UnionNum(0),l)
#define T_lightgrey(l) BuildCont(T_Co_lightgrey,UnionNum(0),l)
#define T_lightgreen(l) BuildCont(T_Co_lightgreen,UnionNum(0),l)
#define T_lightcyan(l) BuildCont(T_Co_lightcyan,UnionNum(0),l)
#define T_lightblue(l) BuildCont(T_Co_lightblue,UnionNum(0),l)
#define T_khaki(l) BuildCont(T_Co_khaki,UnionNum(0),l)
#define T_green(l) BuildCont(T_Co_green,UnionNum(0),l)
#define T_gold(l) BuildCont(T_Co_gold,UnionNum(0),l)
#define T_darkyellow(l) BuildCont(T_Co_darkyellow,UnionNum(0),l)
#define T_darkred(l) BuildCont(T_Co_darkred,UnionNum(0),l)
#define T_darkmagenta(l) BuildCont(T_Co_darkmagenta,UnionNum(0),l)
#define T_darkgrey(l) BuildCont(T_Co_darkgrey,UnionNum(0),l)
#define T_darkgreen(l) BuildCont(T_Co_darkgreen,UnionNum(0),l)
#define T_darkcyan(l) BuildCont(T_Co_darkcyan,UnionNum(0),l)
#define T_darkblue(l) BuildCont(T_Co_darkblue,UnionNum(0),l)
#define T_cyan(l) BuildCont(T_Co_cyan,UnionNum(0),l)
#define T_blue(l) BuildCont(T_Co_blue,UnionNum(0),l)
#define T_black(l) BuildCont(T_Co_black,UnionNum(0),l)
#define T_aquamarine(l) BuildCont(T_Co_aquamarine,UnionNum(0),l)
#define T_yscrollbar(s0,l) BuildTree(T_Co_yscrollbar,1,UnionNum(0),l,s0)
#define T_xscrollbar(s0,l) BuildTree(T_Co_xscrollbar,1,UnionNum(0),l,s0)
#define T_outputfunction(s0,l) BuildTree(T_Co_outputfunction,1,UnionNum(0),l,s0)
#define T_inputfunction(s0,l) BuildTree(T_Co_inputfunction,1,UnionNum(0),l,s0)
#define T_topsort(s0,l) BuildTree(T_Co_topsort,1,UnionNum(0),l,s0)
#define T_layoutparameter(s0,l) BuildTree(T_Co_layoutparameter,1,UnionNum(0),l,s0)
#define T_include(s0,l) BuildTree(T_Co_include,1,UnionNum(0),l,s0)
#define T_typename(s0,l) BuildTree(T_Co_typename,1,UnionNum(0),l,s0)
#define T_straight_max(s0,l) BuildTree(T_Co_straight_max,1,UnionNum(0),l,s0)
#define T_rubber_min(s0,l) BuildTree(T_Co_rubber_min,1,UnionNum(0),l,s0)
#define T_rubber_max(s0,l) BuildTree(T_Co_rubber_max,1,UnionNum(0),l,s0)
#define T_pendel_min(s0,l) BuildTree(T_Co_pendel_min,1,UnionNum(0),l,s0)
#define T_pendel_max(s0,l) BuildTree(T_Co_pendel_max,1,UnionNum(0),l,s0)
#define T_cross_min(s0,l) BuildTree(T_Co_cross_min,1,UnionNum(0),l,s0)
#define T_cross_max(s0,l) BuildTree(T_Co_cross_max,1,UnionNum(0),l,s0)
#define T_bend_max(s0,l) BuildTree(T_Co_bend_max,1,UnionNum(0),l,s0)
#define T_view_splines(s0,l) BuildTree(T_Co_view_splines,1,UnionNum(0),l,s0)
#define T_view_nodes(s0,l) BuildTree(T_Co_view_nodes,1,UnionNum(0),l,s0)
#define T_view_edges(s0,l) BuildTree(T_Co_view_edges,1,UnionNum(0),l,s0)
#define T_view_method(s0,l) BuildTree(T_Co_view_method,1,UnionNum(0),l,s0)
#define T_crossing_weight(s0,l) BuildTree(T_Co_crossing_weight,1,UnionNum(0),l,s0)
#define T_crossing_opt(s0,l) BuildTree(T_Co_crossing_opt,1,UnionNum(0),l,s0)
#define T_crossing_phase2(s0,l) BuildTree(T_Co_crossing_phase2,1,UnionNum(0),l,s0)
#define T_treefactor(s0,l) BuildTree(T_Co_treefactor,1,UnionNum(0),l,s0)
#define T_spreadlevel(s0,l) BuildTree(T_Co_spreadlevel,1,UnionNum(0),l,s0)
#define T_arrow_mode(s0,l) BuildTree(T_Co_arrow_mode,1,UnionNum(0),l,s0)
#define T_port_sharing(s0,l) BuildTree(T_Co_port_sharing,1,UnionNum(0),l,s0)
#define T_node_alignment(s0,l) BuildTree(T_Co_node_alignment,1,UnionNum(0),l,s0)
#define T_orientation(s0,l) BuildTree(T_Co_orientation,1,UnionNum(0),l,s0)
#define T_dummy(l) BuildCont(T_Co_dummy,UnionNum(0),l)
#define T_nonearedges(l) BuildCont(T_Co_nonearedges,UnionNum(0),l)
#define T_smanhatten(s0,l) BuildTree(T_Co_smanhatten,1,UnionNum(0),l,s0)
#define T_manhatten(s0,l) BuildTree(T_Co_manhatten,1,UnionNum(0),l,s0)
#define T_priophase(s0,l) BuildTree(T_Co_priophase,1,UnionNum(0),l,s0)
#define T_straightphase(s0,l) BuildTree(T_Co_straightphase,1,UnionNum(0),l,s0)
#define T_hidesingles(s0,l) BuildTree(T_Co_hidesingles,1,UnionNum(0),l,s0)
#define T_finetuning(s0,l) BuildTree(T_Co_finetuning,1,UnionNum(0),l,s0)
#define T_dirty_edge_label(s0,l) BuildTree(T_Co_dirty_edge_label,1,UnionNum(0),l,s0)
#define T_display_edge_label(s0,l) BuildTree(T_Co_display_edge_label,1,UnionNum(0),l,s0)
#define T_late_edge_label(s0,l) BuildTree(T_Co_late_edge_label,1,UnionNum(0),l,s0)
#define T_splinefactor(s0,l) BuildTree(T_Co_splinefactor,1,UnionNum(0),l,s0)
#define T_nearfactor(s0,l) BuildTree(T_Co_nearfactor,1,UnionNum(0),l,s0)
#define T_upfactor(s0,l) BuildTree(T_Co_upfactor,1,UnionNum(0),l,s0)
#define T_downfactor(s0,l) BuildTree(T_Co_downfactor,1,UnionNum(0),l,s0)
#define T_layoutfrequency(s0,l) BuildTree(T_Co_layoutfrequency,1,UnionNum(0),l,s0)
#define T_layoutalgorithm(s0,l) BuildTree(T_Co_layoutalgorithm,1,UnionNum(0),l,s0)
#define T_colentry(s0,s1,s2,s3,l) BuildTree(T_Co_colentry,4,UnionNum(0),l,s0,s1,s2,s3)
#define T_infoname(s0,s1,l) BuildTree(T_Co_infoname,2,UnionNum(0),l,s0,s1)
#define T_classname(s0,s1,l) BuildTree(T_Co_classname,2,UnionNum(0),l,s0,s1)
#define T_hidden(s0,l) BuildTree(T_Co_hidden,1,UnionNum(0),l,s0)
#define T_yraster(s0,l) BuildTree(T_Co_yraster,1,UnionNum(0),l,s0)
#define T_xlraster(s0,l) BuildTree(T_Co_xlraster,1,UnionNum(0),l,s0)
#define T_xraster(s0,l) BuildTree(T_Co_xraster,1,UnionNum(0),l,s0)
#define T_yspace(s0,l) BuildTree(T_Co_yspace,1,UnionNum(0),l,s0)
#define T_xlspace(s0,l) BuildTree(T_Co_xlspace,1,UnionNum(0),l,s0)
#define T_xspace(s0,l) BuildTree(T_Co_xspace,1,UnionNum(0),l,s0)
#define T_ybase(s0,l) BuildTree(T_Co_ybase,1,UnionNum(0),l,s0)
#define T_xbase(s0,l) BuildTree(T_Co_xbase,1,UnionNum(0),l,s0)
#define T_ymax(s0,l) BuildTree(T_Co_ymax,1,UnionNum(0),l,s0)
#define T_xmax(s0,l) BuildTree(T_Co_xmax,1,UnionNum(0),l,s0)
#define T_status(s0,l) BuildTree(T_Co_status,1,UnionNum(0),l,s0)
#define T_horizontal_order(s0,l) BuildTree(T_Co_horizontal_order,1,UnionNum(0),l,s0)
#define T_level(s0,l) BuildTree(T_Co_level,1,UnionNum(0),l,s0)
#define T_shape(s0,l) BuildTree(T_Co_shape,1,UnionNum(0),l,s0)
#define T_textmode(s0,l) BuildTree(T_Co_textmode,1,UnionNum(0),l,s0)
#define T_stretch(s0,l) BuildTree(T_Co_stretch,1,UnionNum(0),l,s0)
#define T_shrink(s0,l) BuildTree(T_Co_shrink,1,UnionNum(0),l,s0)
#define T_scaling(s0,l) BuildTree(T_Co_scaling,1,UnionNum(0),l,s0)
#define T_folding(s0,l) BuildTree(T_Co_folding,1,UnionNum(0),l,s0)
#define T_loc(s0,s1,l) BuildTree(T_Co_loc,2,UnionNum(0),l,s0,s1)
#define T_ydef(s0,l) BuildTree(T_Co_ydef,1,UnionNum(0),l,s0)
#define T_xdef(s0,l) BuildTree(T_Co_xdef,1,UnionNum(0),l,s0)
#define T_borderwidth(s0,l) BuildTree(T_Co_borderwidth,1,UnionNum(0),l,s0)
#define T_height(s0,l) BuildTree(T_Co_height,1,UnionNum(0),l,s0)
#define T_width(s0,l) BuildTree(T_Co_width,1,UnionNum(0),l,s0)
#define T_colorborder(s0,l) BuildTree(T_Co_colorborder,1,UnionNum(0),l,s0)
#define T_textcolor(s0,l) BuildTree(T_Co_textcolor,1,UnionNum(0),l,s0)
#define T_color(s0,l) BuildTree(T_Co_color,1,UnionNum(0),l,s0)
#define T_info3(s0,l) BuildTree(T_Co_info3,1,UnionNum(0),l,s0)
#define T_info2(s0,l) BuildTree(T_Co_info2,1,UnionNum(0),l,s0)
#define T_info1(s0,l) BuildTree(T_Co_info1,1,UnionNum(0),l,s0)
#define T_label(s0,l) BuildTree(T_Co_label,1,UnionNum(0),l,s0)
#define T_title(s0,l) BuildTree(T_Co_title,1,UnionNum(0),l,s0)
#define T_constraint(s0,l) BuildTree(T_Co_constraint,1,UnionNum(0),l,s0)
#define T_back_edge(s0,l) BuildTree(T_Co_back_edge,1,UnionNum(0),l,s0)
#define T_bent_near_edge(s0,l) BuildTree(T_Co_bent_near_edge,1,UnionNum(0),l,s0)
#define T_near_edge(s0,l) BuildTree(T_Co_near_edge,1,UnionNum(0),l,s0)
#define T_edge(s0,l) BuildTree(T_Co_edge,1,UnionNum(0),l,s0)
#define T_node(s0,l) BuildTree(T_Co_node,1,UnionNum(0),l,s0)
#define T_graph(s0,l) BuildTree(T_Co_graph,1,UnionNum(0),l,s0)
#define T_foldedge_defaults(s0,l) BuildTree(T_Co_foldedge_defaults,1,UnionNum(0),l,s0)
#define T_foldnode_defaults(s0,l) BuildTree(T_Co_foldnode_defaults,1,UnionNum(0),l,s0)
#define T_edge_defaults(s0,l) BuildTree(T_Co_edge_defaults,1,UnionNum(0),l,s0)
#define T_node_defaults(s0,l) BuildTree(T_Co_node_defaults,1,UnionNum(0),l,s0)
#define T_graph_attribute(s0,l) BuildTree(T_Co_graph_attribute,1,UnionNum(0),l,s0)
#define T_graph_entry(s0,s1,l) BuildTree(T_Co_graph_entry,2,UnionNum(0),l,s0,s1)


#line 24 "/RW/esprit/users/sander/src/PARSEGEN/help.skel"


#line 1 "/RW/esprit/users/sander/src/PARSEGEN/stdth.skel"
 
#ifndef STDHASH
#define STDHASH

/* $Id$ */

#undef  HASHGENSTD
#define HASHGENSTD


/*--------------------------------------------------------------------*/
/*  Standard Hash Table Routines                                      */
/*--------------------------------------------------------------------*/

/* Global Variables */
/*------------------*/

extern long   table_size;

#ifndef PARSEGENSTD

#ifndef ALIGN
#define ALIGN 8
#define IALIGN (ALIGN-1)
#endif
#ifndef STRINGBLOCKSIZE
#define STRINGBLOCKSIZE 5000
#endif

#ifdef ANSI_C
char *StringHeapMalloc(int x);
void StringHeapFree(void);
else
char *StringHeapMalloc();
void StringHeapFree();
#endif

#endif /* PARSEGENSTD */

#ifndef hash_size
#define hash_size 211
#endif
#ifndef hashtable_size
#define hashtable_size 10000L
#endif


#ifdef ANSI_C
void FreeHash(void);
long HashInsert(register char *s);
long HashTableSize(void);
char *Decode(long x);
#else
void FreeHash();
long HashInsert();
long HashTableSize();
char *Decode();
#endif /* ANSI_C */

#endif /* STDHASH */

/*-- end of standard hash table interface ----------------------------*/
 


#line 24 "/RW/esprit/users/sander/src/PARSEGEN/help.skel"

extern yysyntaxtree Syntax_Tree;
extern int nr_errors;

#ifdef ANSI_C
int parse(void);
#else
int parse();
#endif

#endif  /* SCANPARSE_H */

