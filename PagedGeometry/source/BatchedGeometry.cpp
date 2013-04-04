



<!DOCTYPE html>
<html>
<head>
 <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" >
 <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1" >
 
 <meta name="ROBOTS" content="NOARCHIVE">
 
 <link rel="icon" type="image/vnd.microsoft.icon" href="http://www.gstatic.com/codesite/ph/images/phosting.ico">
 
 
 <script type="text/javascript">
 
 (function(){(function(){function b(a){this.t={};this.tick=function(a,b,c){b=void 0!=c?c:(new Date).getTime();this.t[a]=b};this.tick("start",null,a)}var a=new b;window.jstiming={Timer:b,load:a};try{a=null,window.chrome&&window.chrome.csi&&(a=Math.floor(window.chrome.csi().pageT)),null==a&&window.gtbExternal&&(a=window.gtbExternal.pageT()),null==a&&window.external&&(a=window.external.pageT),a&&(window.jstiming.pt=a)}catch(d){}})();})();

 
 
 
 
 var codesite_token = "BKkLBik2J6v6PYc0DwT3gu89D9I:1365060230755";
 
 
 var CS_env = {"token":"BKkLBik2J6v6PYc0DwT3gu89D9I:1365060230755","assetVersionPath":"http://www.gstatic.com/codesite/ph/15170358673760952803","projectHomeUrl":"/p/ogre-paged","profileUrl":"/u/107319084838887332900/","projectName":"ogre-paged","domainName":null,"loggedInUserEmail":"GeoffTopping@gmail.com","relativeBaseUrl":"","assetHostPath":"http://www.gstatic.com/codesite/ph"};
 var _gaq = _gaq || [];
 _gaq.push(
 ['siteTracker._setAccount', 'UA-18071-1'],
 ['siteTracker._trackPageview']);
 
 (function() {
 var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
 ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
 (document.getElementsByTagName('head')[0] || document.getElementsByTagName('body')[0]).appendChild(ga);
 })();
 
 </script>
 
 
 <title>BatchedGeometry.cpp - 
 ogre-paged -
 
 
 Paged Geometry for Ogre3D - Google Project Hosting
 </title>
 <link type="text/css" rel="stylesheet" href="http://www.gstatic.com/codesite/ph/15170358673760952803/css/core.css">
 
 <link type="text/css" rel="stylesheet" href="http://www.gstatic.com/codesite/ph/15170358673760952803/css/ph_detail.css" >
 
 
 <link type="text/css" rel="stylesheet" href="http://www.gstatic.com/codesite/ph/15170358673760952803/css/d_sb.css" >
 
 
 
<!--[if IE]>
 <link type="text/css" rel="stylesheet" href="http://www.gstatic.com/codesite/ph/15170358673760952803/css/d_ie.css" >
<![endif]-->
 <style type="text/css">
 .menuIcon.off { background: no-repeat url(http://www.gstatic.com/codesite/ph/images/dropdown_sprite.gif) 0 -42px }
 .menuIcon.on { background: no-repeat url(http://www.gstatic.com/codesite/ph/images/dropdown_sprite.gif) 0 -28px }
 .menuIcon.down { background: no-repeat url(http://www.gstatic.com/codesite/ph/images/dropdown_sprite.gif) 0 0; }
 
 
 
  tr.inline_comment {
 background: #fff;
 vertical-align: top;
 }
 div.draft, div.published {
 padding: .3em;
 border: 1px solid #999; 
 margin-bottom: .1em;
 font-family: arial, sans-serif;
 max-width: 60em;
 }
 div.draft {
 background: #ffa;
 } 
 div.published {
 background: #e5ecf9;
 }
 div.published .body, div.draft .body {
 padding: .5em .1em .1em .1em;
 max-width: 60em;
 white-space: pre-wrap;
 white-space: -moz-pre-wrap;
 white-space: -pre-wrap;
 white-space: -o-pre-wrap;
 word-wrap: break-word;
 font-size: 1em;
 }
 div.draft .actions {
 margin-left: 1em;
 font-size: 90%;
 }
 div.draft form {
 padding: .5em .5em .5em 0;
 }
 div.draft textarea, div.published textarea {
 width: 95%;
 height: 10em;
 font-family: arial, sans-serif;
 margin-bottom: .5em;
 }

 
 .nocursor, .nocursor td, .cursor_hidden, .cursor_hidden td {
 background-color: white;
 height: 2px;
 }
 .cursor, .cursor td {
 background-color: darkblue;
 height: 2px;
 display: '';
 }
 
 
.list {
 border: 1px solid white;
 border-bottom: 0;
}

 
 </style>
</head>
<body class="t4">
<script type="text/javascript">
 window.___gcfg = {lang: 'en'};
 (function() 
 {var po = document.createElement("script");
 po.type = "text/javascript"; po.async = true;po.src = "https://apis.google.com/js/plusone.js";
 var s = document.getElementsByTagName("script")[0];
 s.parentNode.insertBefore(po, s);
 })();
</script>
<div class="headbg">

 <div id="gaia">
 

 <span>
 
 
 
 <a href="#" id="multilogin-dropdown" onclick="return false;"
 ><u><b>GeoffTopping@gmail.com</b></u> <small>&#9660;</small></a>
 
 
 | <a href="/u/107319084838887332900/" id="projects-dropdown" onclick="return false;"
 ><u>My favorites</u> <small>&#9660;</small></a>
 | <a href="/u/107319084838887332900/" onclick="_CS_click('/gb/ph/profile');"
 title="Profile, Updates, and Settings"
 ><u>Profile</u></a>
 | <a href="https://www.google.com/accounts/Logout?continue=http%3A%2F%2Fcode.google.com%2Fp%2Fogre-paged%2Fsource%2Fbrowse%2Fsource%2FBatchedGeometry.cpp" 
 onclick="_CS_click('/gb/ph/signout');"
 ><u>Sign out</u></a>
 
 </span>

 </div>

 <div class="gbh" style="left: 0pt;"></div>
 <div class="gbh" style="right: 0pt;"></div>
 
 
 <div style="height: 1px"></div>
<!--[if lte IE 7]>
<div style="text-align:center;">
Your version of Internet Explorer is not supported. Try a browser that
contributes to open source, such as <a href="http://www.firefox.com">Firefox</a>,
<a href="http://www.google.com/chrome">Google Chrome</a>, or
<a href="http://code.google.com/chrome/chromeframe/">Google Chrome Frame</a>.
</div>
<![endif]-->



 <table style="padding:0px; margin: 0px 0px 10px 0px; width:100%" cellpadding="0" cellspacing="0"
 itemscope itemtype="http://schema.org/CreativeWork">
 <tr style="height: 58px;">
 
 
 
 <td id="plogo">
 <link itemprop="url" href="/p/ogre-paged">
 <a href="/p/ogre-paged/">
 
 <img src="http://www.gstatic.com/codesite/ph/images/defaultlogo.png" alt="Logo" itemprop="image">
 
 </a>
 </td>
 
 <td style="padding-left: 0.5em">
 
 <div id="pname">
 <a href="/p/ogre-paged/"><span itemprop="name">ogre-paged</span></a>
 </div>
 
 <div id="psum">
 <a id="project_summary_link"
 href="/p/ogre-paged/"><span itemprop="description">Paged Geometry for Ogre3D</span></a>
 
 </div>
 
 
 </td>
 <td style="white-space:nowrap;text-align:right; vertical-align:bottom;">
 
 <form action="/hosting/search">
 <input size="30" name="q" value="" type="text">
 
 <input type="submit" name="projectsearch" value="Search projects" >
 </form>
 
 </tr>
 </table>

</div>

 
<div id="mt" class="gtb"> 
 <a href="/p/ogre-paged/" class="tab ">Project&nbsp;Home</a>
 
 
 
 
 <a href="/p/ogre-paged/downloads/list" class="tab ">Downloads</a>
 
 
 
 
 
 <a href="/p/ogre-paged/w/list" class="tab ">Wiki</a>
 
 
 
 
 
 <a href="/p/ogre-paged/issues/list"
 class="tab ">Issues</a>
 
 
 
 
 
 <a href="/p/ogre-paged/source/checkout"
 class="tab active">Source</a>
 
 
 
 
 
 
 
 
 <div class=gtbc></div>
</div>
<table cellspacing="0" cellpadding="0" width="100%" align="center" border="0" class="st">
 <tr>
 
 
 
 
 
 
 <td class="subt">
 <div class="st2">
 <div class="isf">
 
 <form action="/p/ogre-paged/source/browse" style="display: inline">
 
 Repository:
 <select name="repo" id="repo" style="font-size: 92%" onchange="submit()">
 <option value="default">default</option><option value="wiki">wiki</option>
 </select>
 </form>
 
 


 <span class="inst1"><a href="/p/ogre-paged/source/checkout">Checkout</a></span> &nbsp;
 <span class="inst2"><a href="/p/ogre-paged/source/browse/">Browse</a></span> &nbsp;
 <span class="inst3"><a href="/p/ogre-paged/source/list">Changes</a></span> &nbsp;
 <span class="inst4"><a href="/p/ogre-paged/source/clones">Clones</a></span> &nbsp; 
 
 
 
 
 
 
 </form>
 <script type="text/javascript">
 
 function codesearchQuery(form) {
 var query = document.getElementById('q').value;
 if (query) { form.action += '%20' + query; }
 }
 </script>
 </div>
</div>

 </td>
 
 
 
 <td align="right" valign="top" class="bevel-right"></td>
 </tr>
</table>


<script type="text/javascript">
 var cancelBubble = false;
 function _go(url) { document.location = url; }
</script>
<div id="maincol"
 
>

 




<div class="collapse">
<div id="colcontrol">
<style type="text/css">
 #file_flipper { white-space: nowrap; padding-right: 2em; }
 #file_flipper.hidden { display: none; }
 #file_flipper .pagelink { color: #0000CC; text-decoration: underline; }
 #file_flipper #visiblefiles { padding-left: 0.5em; padding-right: 0.5em; }
</style>
<table id="nav_and_rev" class="list"
 cellpadding="0" cellspacing="0" width="100%">
 <tr>
 
 <td nowrap="nowrap" class="src_crumbs src_nav" width="33%">
 <strong class="src_nav">Source path:&nbsp;</strong>
 <span id="crumb_root">
 
 <a href="/p/ogre-paged/source/browse/">hg</a>/&nbsp;</span>
 <span id="crumb_links" class="ifClosed"><a href="/p/ogre-paged/source/browse/source/">source</a><span class="sp">/&nbsp;</span>BatchedGeometry.cpp</span>
 
 


 </td>
 
 
 <td nowrap="nowrap" width="33%" align="center">
 <a href="/p/ogre-paged/source/browse/source/BatchedGeometry.cpp?edit=1"
 ><img src="http://www.gstatic.com/codesite/ph/images/pencil-y14.png"
 class="edit_icon">Edit file</a>
 </td>
 
 
 <td nowrap="nowrap" width="33%" align="right">
 <table cellpadding="0" cellspacing="0" style="font-size: 100%"><tr>
 
 
 <td class="flipper">
 <ul class="leftside">
 
 <li><a href="/p/ogre-paged/source/browse/source/BatchedGeometry.cpp?r=4b9c9c16a1d37aebb2a47614615e6809d17b5ae2" title="Previous">&lsaquo;4b9c9c16a1d3</a></li>
 
 </ul>
 </td>
 
 <td class="flipper"><b>06de7ef9f790</b></td>
 
 </tr></table>
 </td> 
 </tr>
</table>

<div class="fc">
 
 
 
<style type="text/css">
.undermouse span {
 background-image: url(http://www.gstatic.com/codesite/ph/images/comments.gif); }
</style>
<table class="opened" id="review_comment_area"
onmouseout="gutterOut()"><tr>
<td id="nums">
<pre><table width="100%"><tr class="nocursor"><td></td></tr></table></pre>
<pre><table width="100%" id="nums_table_0"><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1"

 onmouseover="gutterOver(1)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1);">&nbsp;</span
></td><td id="1"><a href="#1">1</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_2"

 onmouseover="gutterOver(2)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',2);">&nbsp;</span
></td><td id="2"><a href="#2">2</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_3"

 onmouseover="gutterOver(3)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',3);">&nbsp;</span
></td><td id="3"><a href="#3">3</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_4"

 onmouseover="gutterOver(4)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',4);">&nbsp;</span
></td><td id="4"><a href="#4">4</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_5"

 onmouseover="gutterOver(5)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',5);">&nbsp;</span
></td><td id="5"><a href="#5">5</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_6"

 onmouseover="gutterOver(6)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',6);">&nbsp;</span
></td><td id="6"><a href="#6">6</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_7"

 onmouseover="gutterOver(7)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',7);">&nbsp;</span
></td><td id="7"><a href="#7">7</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_8"

 onmouseover="gutterOver(8)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',8);">&nbsp;</span
></td><td id="8"><a href="#8">8</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_9"

 onmouseover="gutterOver(9)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',9);">&nbsp;</span
></td><td id="9"><a href="#9">9</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_10"

 onmouseover="gutterOver(10)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',10);">&nbsp;</span
></td><td id="10"><a href="#10">10</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_11"

 onmouseover="gutterOver(11)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',11);">&nbsp;</span
></td><td id="11"><a href="#11">11</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_12"

 onmouseover="gutterOver(12)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',12);">&nbsp;</span
></td><td id="12"><a href="#12">12</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_13"

 onmouseover="gutterOver(13)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',13);">&nbsp;</span
></td><td id="13"><a href="#13">13</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_14"

 onmouseover="gutterOver(14)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',14);">&nbsp;</span
></td><td id="14"><a href="#14">14</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_15"

 onmouseover="gutterOver(15)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',15);">&nbsp;</span
></td><td id="15"><a href="#15">15</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_16"

 onmouseover="gutterOver(16)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',16);">&nbsp;</span
></td><td id="16"><a href="#16">16</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_17"

 onmouseover="gutterOver(17)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',17);">&nbsp;</span
></td><td id="17"><a href="#17">17</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_18"

 onmouseover="gutterOver(18)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',18);">&nbsp;</span
></td><td id="18"><a href="#18">18</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_19"

 onmouseover="gutterOver(19)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',19);">&nbsp;</span
></td><td id="19"><a href="#19">19</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_20"

 onmouseover="gutterOver(20)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',20);">&nbsp;</span
></td><td id="20"><a href="#20">20</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_21"

 onmouseover="gutterOver(21)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',21);">&nbsp;</span
></td><td id="21"><a href="#21">21</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_22"

 onmouseover="gutterOver(22)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',22);">&nbsp;</span
></td><td id="22"><a href="#22">22</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_23"

 onmouseover="gutterOver(23)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',23);">&nbsp;</span
></td><td id="23"><a href="#23">23</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_24"

 onmouseover="gutterOver(24)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',24);">&nbsp;</span
></td><td id="24"><a href="#24">24</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_25"

 onmouseover="gutterOver(25)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',25);">&nbsp;</span
></td><td id="25"><a href="#25">25</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_26"

 onmouseover="gutterOver(26)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',26);">&nbsp;</span
></td><td id="26"><a href="#26">26</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_27"

 onmouseover="gutterOver(27)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',27);">&nbsp;</span
></td><td id="27"><a href="#27">27</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_28"

 onmouseover="gutterOver(28)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',28);">&nbsp;</span
></td><td id="28"><a href="#28">28</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_29"

 onmouseover="gutterOver(29)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',29);">&nbsp;</span
></td><td id="29"><a href="#29">29</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_30"

 onmouseover="gutterOver(30)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',30);">&nbsp;</span
></td><td id="30"><a href="#30">30</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_31"

 onmouseover="gutterOver(31)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',31);">&nbsp;</span
></td><td id="31"><a href="#31">31</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_32"

 onmouseover="gutterOver(32)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',32);">&nbsp;</span
></td><td id="32"><a href="#32">32</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_33"

 onmouseover="gutterOver(33)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',33);">&nbsp;</span
></td><td id="33"><a href="#33">33</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_34"

 onmouseover="gutterOver(34)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',34);">&nbsp;</span
></td><td id="34"><a href="#34">34</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_35"

 onmouseover="gutterOver(35)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',35);">&nbsp;</span
></td><td id="35"><a href="#35">35</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_36"

 onmouseover="gutterOver(36)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',36);">&nbsp;</span
></td><td id="36"><a href="#36">36</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_37"

 onmouseover="gutterOver(37)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',37);">&nbsp;</span
></td><td id="37"><a href="#37">37</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_38"

 onmouseover="gutterOver(38)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',38);">&nbsp;</span
></td><td id="38"><a href="#38">38</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_39"

 onmouseover="gutterOver(39)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',39);">&nbsp;</span
></td><td id="39"><a href="#39">39</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_40"

 onmouseover="gutterOver(40)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',40);">&nbsp;</span
></td><td id="40"><a href="#40">40</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_41"

 onmouseover="gutterOver(41)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',41);">&nbsp;</span
></td><td id="41"><a href="#41">41</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_42"

 onmouseover="gutterOver(42)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',42);">&nbsp;</span
></td><td id="42"><a href="#42">42</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_43"

 onmouseover="gutterOver(43)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',43);">&nbsp;</span
></td><td id="43"><a href="#43">43</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_44"

 onmouseover="gutterOver(44)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',44);">&nbsp;</span
></td><td id="44"><a href="#44">44</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_45"

 onmouseover="gutterOver(45)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',45);">&nbsp;</span
></td><td id="45"><a href="#45">45</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_46"

 onmouseover="gutterOver(46)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',46);">&nbsp;</span
></td><td id="46"><a href="#46">46</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_47"

 onmouseover="gutterOver(47)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',47);">&nbsp;</span
></td><td id="47"><a href="#47">47</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_48"

 onmouseover="gutterOver(48)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',48);">&nbsp;</span
></td><td id="48"><a href="#48">48</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_49"

 onmouseover="gutterOver(49)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',49);">&nbsp;</span
></td><td id="49"><a href="#49">49</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_50"

 onmouseover="gutterOver(50)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',50);">&nbsp;</span
></td><td id="50"><a href="#50">50</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_51"

 onmouseover="gutterOver(51)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',51);">&nbsp;</span
></td><td id="51"><a href="#51">51</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_52"

 onmouseover="gutterOver(52)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',52);">&nbsp;</span
></td><td id="52"><a href="#52">52</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_53"

 onmouseover="gutterOver(53)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',53);">&nbsp;</span
></td><td id="53"><a href="#53">53</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_54"

 onmouseover="gutterOver(54)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',54);">&nbsp;</span
></td><td id="54"><a href="#54">54</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_55"

 onmouseover="gutterOver(55)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',55);">&nbsp;</span
></td><td id="55"><a href="#55">55</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_56"

 onmouseover="gutterOver(56)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',56);">&nbsp;</span
></td><td id="56"><a href="#56">56</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_57"

 onmouseover="gutterOver(57)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',57);">&nbsp;</span
></td><td id="57"><a href="#57">57</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_58"

 onmouseover="gutterOver(58)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',58);">&nbsp;</span
></td><td id="58"><a href="#58">58</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_59"

 onmouseover="gutterOver(59)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',59);">&nbsp;</span
></td><td id="59"><a href="#59">59</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_60"

 onmouseover="gutterOver(60)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',60);">&nbsp;</span
></td><td id="60"><a href="#60">60</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_61"

 onmouseover="gutterOver(61)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',61);">&nbsp;</span
></td><td id="61"><a href="#61">61</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_62"

 onmouseover="gutterOver(62)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',62);">&nbsp;</span
></td><td id="62"><a href="#62">62</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_63"

 onmouseover="gutterOver(63)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',63);">&nbsp;</span
></td><td id="63"><a href="#63">63</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_64"

 onmouseover="gutterOver(64)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',64);">&nbsp;</span
></td><td id="64"><a href="#64">64</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_65"

 onmouseover="gutterOver(65)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',65);">&nbsp;</span
></td><td id="65"><a href="#65">65</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_66"

 onmouseover="gutterOver(66)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',66);">&nbsp;</span
></td><td id="66"><a href="#66">66</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_67"

 onmouseover="gutterOver(67)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',67);">&nbsp;</span
></td><td id="67"><a href="#67">67</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_68"

 onmouseover="gutterOver(68)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',68);">&nbsp;</span
></td><td id="68"><a href="#68">68</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_69"

 onmouseover="gutterOver(69)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',69);">&nbsp;</span
></td><td id="69"><a href="#69">69</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_70"

 onmouseover="gutterOver(70)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',70);">&nbsp;</span
></td><td id="70"><a href="#70">70</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_71"

 onmouseover="gutterOver(71)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',71);">&nbsp;</span
></td><td id="71"><a href="#71">71</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_72"

 onmouseover="gutterOver(72)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',72);">&nbsp;</span
></td><td id="72"><a href="#72">72</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_73"

 onmouseover="gutterOver(73)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',73);">&nbsp;</span
></td><td id="73"><a href="#73">73</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_74"

 onmouseover="gutterOver(74)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',74);">&nbsp;</span
></td><td id="74"><a href="#74">74</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_75"

 onmouseover="gutterOver(75)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',75);">&nbsp;</span
></td><td id="75"><a href="#75">75</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_76"

 onmouseover="gutterOver(76)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',76);">&nbsp;</span
></td><td id="76"><a href="#76">76</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_77"

 onmouseover="gutterOver(77)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',77);">&nbsp;</span
></td><td id="77"><a href="#77">77</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_78"

 onmouseover="gutterOver(78)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',78);">&nbsp;</span
></td><td id="78"><a href="#78">78</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_79"

 onmouseover="gutterOver(79)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',79);">&nbsp;</span
></td><td id="79"><a href="#79">79</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_80"

 onmouseover="gutterOver(80)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',80);">&nbsp;</span
></td><td id="80"><a href="#80">80</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_81"

 onmouseover="gutterOver(81)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',81);">&nbsp;</span
></td><td id="81"><a href="#81">81</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_82"

 onmouseover="gutterOver(82)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',82);">&nbsp;</span
></td><td id="82"><a href="#82">82</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_83"

 onmouseover="gutterOver(83)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',83);">&nbsp;</span
></td><td id="83"><a href="#83">83</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_84"

 onmouseover="gutterOver(84)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',84);">&nbsp;</span
></td><td id="84"><a href="#84">84</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_85"

 onmouseover="gutterOver(85)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',85);">&nbsp;</span
></td><td id="85"><a href="#85">85</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_86"

 onmouseover="gutterOver(86)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',86);">&nbsp;</span
></td><td id="86"><a href="#86">86</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_87"

 onmouseover="gutterOver(87)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',87);">&nbsp;</span
></td><td id="87"><a href="#87">87</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_88"

 onmouseover="gutterOver(88)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',88);">&nbsp;</span
></td><td id="88"><a href="#88">88</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_89"

 onmouseover="gutterOver(89)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',89);">&nbsp;</span
></td><td id="89"><a href="#89">89</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_90"

 onmouseover="gutterOver(90)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',90);">&nbsp;</span
></td><td id="90"><a href="#90">90</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_91"

 onmouseover="gutterOver(91)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',91);">&nbsp;</span
></td><td id="91"><a href="#91">91</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_92"

 onmouseover="gutterOver(92)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',92);">&nbsp;</span
></td><td id="92"><a href="#92">92</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_93"

 onmouseover="gutterOver(93)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',93);">&nbsp;</span
></td><td id="93"><a href="#93">93</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_94"

 onmouseover="gutterOver(94)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',94);">&nbsp;</span
></td><td id="94"><a href="#94">94</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_95"

 onmouseover="gutterOver(95)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',95);">&nbsp;</span
></td><td id="95"><a href="#95">95</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_96"

 onmouseover="gutterOver(96)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',96);">&nbsp;</span
></td><td id="96"><a href="#96">96</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_97"

 onmouseover="gutterOver(97)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',97);">&nbsp;</span
></td><td id="97"><a href="#97">97</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_98"

 onmouseover="gutterOver(98)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',98);">&nbsp;</span
></td><td id="98"><a href="#98">98</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_99"

 onmouseover="gutterOver(99)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',99);">&nbsp;</span
></td><td id="99"><a href="#99">99</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_100"

 onmouseover="gutterOver(100)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',100);">&nbsp;</span
></td><td id="100"><a href="#100">100</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_101"

 onmouseover="gutterOver(101)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',101);">&nbsp;</span
></td><td id="101"><a href="#101">101</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_102"

 onmouseover="gutterOver(102)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',102);">&nbsp;</span
></td><td id="102"><a href="#102">102</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_103"

 onmouseover="gutterOver(103)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',103);">&nbsp;</span
></td><td id="103"><a href="#103">103</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_104"

 onmouseover="gutterOver(104)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',104);">&nbsp;</span
></td><td id="104"><a href="#104">104</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_105"

 onmouseover="gutterOver(105)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',105);">&nbsp;</span
></td><td id="105"><a href="#105">105</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_106"

 onmouseover="gutterOver(106)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',106);">&nbsp;</span
></td><td id="106"><a href="#106">106</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_107"

 onmouseover="gutterOver(107)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',107);">&nbsp;</span
></td><td id="107"><a href="#107">107</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_108"

 onmouseover="gutterOver(108)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',108);">&nbsp;</span
></td><td id="108"><a href="#108">108</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_109"

 onmouseover="gutterOver(109)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',109);">&nbsp;</span
></td><td id="109"><a href="#109">109</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_110"

 onmouseover="gutterOver(110)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',110);">&nbsp;</span
></td><td id="110"><a href="#110">110</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_111"

 onmouseover="gutterOver(111)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',111);">&nbsp;</span
></td><td id="111"><a href="#111">111</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_112"

 onmouseover="gutterOver(112)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',112);">&nbsp;</span
></td><td id="112"><a href="#112">112</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_113"

 onmouseover="gutterOver(113)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',113);">&nbsp;</span
></td><td id="113"><a href="#113">113</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_114"

 onmouseover="gutterOver(114)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',114);">&nbsp;</span
></td><td id="114"><a href="#114">114</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_115"

 onmouseover="gutterOver(115)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',115);">&nbsp;</span
></td><td id="115"><a href="#115">115</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_116"

 onmouseover="gutterOver(116)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',116);">&nbsp;</span
></td><td id="116"><a href="#116">116</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_117"

 onmouseover="gutterOver(117)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',117);">&nbsp;</span
></td><td id="117"><a href="#117">117</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_118"

 onmouseover="gutterOver(118)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',118);">&nbsp;</span
></td><td id="118"><a href="#118">118</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_119"

 onmouseover="gutterOver(119)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',119);">&nbsp;</span
></td><td id="119"><a href="#119">119</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_120"

 onmouseover="gutterOver(120)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',120);">&nbsp;</span
></td><td id="120"><a href="#120">120</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_121"

 onmouseover="gutterOver(121)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',121);">&nbsp;</span
></td><td id="121"><a href="#121">121</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_122"

 onmouseover="gutterOver(122)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',122);">&nbsp;</span
></td><td id="122"><a href="#122">122</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_123"

 onmouseover="gutterOver(123)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',123);">&nbsp;</span
></td><td id="123"><a href="#123">123</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_124"

 onmouseover="gutterOver(124)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',124);">&nbsp;</span
></td><td id="124"><a href="#124">124</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_125"

 onmouseover="gutterOver(125)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',125);">&nbsp;</span
></td><td id="125"><a href="#125">125</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_126"

 onmouseover="gutterOver(126)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',126);">&nbsp;</span
></td><td id="126"><a href="#126">126</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_127"

 onmouseover="gutterOver(127)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',127);">&nbsp;</span
></td><td id="127"><a href="#127">127</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_128"

 onmouseover="gutterOver(128)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',128);">&nbsp;</span
></td><td id="128"><a href="#128">128</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_129"

 onmouseover="gutterOver(129)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',129);">&nbsp;</span
></td><td id="129"><a href="#129">129</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_130"

 onmouseover="gutterOver(130)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',130);">&nbsp;</span
></td><td id="130"><a href="#130">130</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_131"

 onmouseover="gutterOver(131)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',131);">&nbsp;</span
></td><td id="131"><a href="#131">131</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_132"

 onmouseover="gutterOver(132)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',132);">&nbsp;</span
></td><td id="132"><a href="#132">132</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_133"

 onmouseover="gutterOver(133)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',133);">&nbsp;</span
></td><td id="133"><a href="#133">133</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_134"

 onmouseover="gutterOver(134)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',134);">&nbsp;</span
></td><td id="134"><a href="#134">134</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_135"

 onmouseover="gutterOver(135)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',135);">&nbsp;</span
></td><td id="135"><a href="#135">135</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_136"

 onmouseover="gutterOver(136)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',136);">&nbsp;</span
></td><td id="136"><a href="#136">136</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_137"

 onmouseover="gutterOver(137)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',137);">&nbsp;</span
></td><td id="137"><a href="#137">137</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_138"

 onmouseover="gutterOver(138)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',138);">&nbsp;</span
></td><td id="138"><a href="#138">138</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_139"

 onmouseover="gutterOver(139)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',139);">&nbsp;</span
></td><td id="139"><a href="#139">139</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_140"

 onmouseover="gutterOver(140)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',140);">&nbsp;</span
></td><td id="140"><a href="#140">140</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_141"

 onmouseover="gutterOver(141)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',141);">&nbsp;</span
></td><td id="141"><a href="#141">141</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_142"

 onmouseover="gutterOver(142)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',142);">&nbsp;</span
></td><td id="142"><a href="#142">142</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_143"

 onmouseover="gutterOver(143)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',143);">&nbsp;</span
></td><td id="143"><a href="#143">143</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_144"

 onmouseover="gutterOver(144)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',144);">&nbsp;</span
></td><td id="144"><a href="#144">144</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_145"

 onmouseover="gutterOver(145)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',145);">&nbsp;</span
></td><td id="145"><a href="#145">145</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_146"

 onmouseover="gutterOver(146)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',146);">&nbsp;</span
></td><td id="146"><a href="#146">146</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_147"

 onmouseover="gutterOver(147)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',147);">&nbsp;</span
></td><td id="147"><a href="#147">147</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_148"

 onmouseover="gutterOver(148)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',148);">&nbsp;</span
></td><td id="148"><a href="#148">148</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_149"

 onmouseover="gutterOver(149)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',149);">&nbsp;</span
></td><td id="149"><a href="#149">149</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_150"

 onmouseover="gutterOver(150)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',150);">&nbsp;</span
></td><td id="150"><a href="#150">150</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_151"

 onmouseover="gutterOver(151)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',151);">&nbsp;</span
></td><td id="151"><a href="#151">151</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_152"

 onmouseover="gutterOver(152)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',152);">&nbsp;</span
></td><td id="152"><a href="#152">152</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_153"

 onmouseover="gutterOver(153)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',153);">&nbsp;</span
></td><td id="153"><a href="#153">153</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_154"

 onmouseover="gutterOver(154)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',154);">&nbsp;</span
></td><td id="154"><a href="#154">154</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_155"

 onmouseover="gutterOver(155)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',155);">&nbsp;</span
></td><td id="155"><a href="#155">155</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_156"

 onmouseover="gutterOver(156)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',156);">&nbsp;</span
></td><td id="156"><a href="#156">156</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_157"

 onmouseover="gutterOver(157)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',157);">&nbsp;</span
></td><td id="157"><a href="#157">157</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_158"

 onmouseover="gutterOver(158)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',158);">&nbsp;</span
></td><td id="158"><a href="#158">158</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_159"

 onmouseover="gutterOver(159)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',159);">&nbsp;</span
></td><td id="159"><a href="#159">159</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_160"

 onmouseover="gutterOver(160)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',160);">&nbsp;</span
></td><td id="160"><a href="#160">160</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_161"

 onmouseover="gutterOver(161)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',161);">&nbsp;</span
></td><td id="161"><a href="#161">161</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_162"

 onmouseover="gutterOver(162)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',162);">&nbsp;</span
></td><td id="162"><a href="#162">162</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_163"

 onmouseover="gutterOver(163)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',163);">&nbsp;</span
></td><td id="163"><a href="#163">163</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_164"

 onmouseover="gutterOver(164)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',164);">&nbsp;</span
></td><td id="164"><a href="#164">164</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_165"

 onmouseover="gutterOver(165)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',165);">&nbsp;</span
></td><td id="165"><a href="#165">165</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_166"

 onmouseover="gutterOver(166)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',166);">&nbsp;</span
></td><td id="166"><a href="#166">166</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_167"

 onmouseover="gutterOver(167)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',167);">&nbsp;</span
></td><td id="167"><a href="#167">167</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_168"

 onmouseover="gutterOver(168)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',168);">&nbsp;</span
></td><td id="168"><a href="#168">168</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_169"

 onmouseover="gutterOver(169)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',169);">&nbsp;</span
></td><td id="169"><a href="#169">169</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_170"

 onmouseover="gutterOver(170)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',170);">&nbsp;</span
></td><td id="170"><a href="#170">170</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_171"

 onmouseover="gutterOver(171)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',171);">&nbsp;</span
></td><td id="171"><a href="#171">171</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_172"

 onmouseover="gutterOver(172)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',172);">&nbsp;</span
></td><td id="172"><a href="#172">172</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_173"

 onmouseover="gutterOver(173)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',173);">&nbsp;</span
></td><td id="173"><a href="#173">173</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_174"

 onmouseover="gutterOver(174)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',174);">&nbsp;</span
></td><td id="174"><a href="#174">174</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_175"

 onmouseover="gutterOver(175)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',175);">&nbsp;</span
></td><td id="175"><a href="#175">175</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_176"

 onmouseover="gutterOver(176)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',176);">&nbsp;</span
></td><td id="176"><a href="#176">176</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_177"

 onmouseover="gutterOver(177)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',177);">&nbsp;</span
></td><td id="177"><a href="#177">177</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_178"

 onmouseover="gutterOver(178)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',178);">&nbsp;</span
></td><td id="178"><a href="#178">178</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_179"

 onmouseover="gutterOver(179)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',179);">&nbsp;</span
></td><td id="179"><a href="#179">179</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_180"

 onmouseover="gutterOver(180)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',180);">&nbsp;</span
></td><td id="180"><a href="#180">180</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_181"

 onmouseover="gutterOver(181)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',181);">&nbsp;</span
></td><td id="181"><a href="#181">181</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_182"

 onmouseover="gutterOver(182)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',182);">&nbsp;</span
></td><td id="182"><a href="#182">182</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_183"

 onmouseover="gutterOver(183)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',183);">&nbsp;</span
></td><td id="183"><a href="#183">183</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_184"

 onmouseover="gutterOver(184)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',184);">&nbsp;</span
></td><td id="184"><a href="#184">184</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_185"

 onmouseover="gutterOver(185)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',185);">&nbsp;</span
></td><td id="185"><a href="#185">185</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_186"

 onmouseover="gutterOver(186)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',186);">&nbsp;</span
></td><td id="186"><a href="#186">186</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_187"

 onmouseover="gutterOver(187)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',187);">&nbsp;</span
></td><td id="187"><a href="#187">187</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_188"

 onmouseover="gutterOver(188)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',188);">&nbsp;</span
></td><td id="188"><a href="#188">188</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_189"

 onmouseover="gutterOver(189)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',189);">&nbsp;</span
></td><td id="189"><a href="#189">189</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_190"

 onmouseover="gutterOver(190)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',190);">&nbsp;</span
></td><td id="190"><a href="#190">190</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_191"

 onmouseover="gutterOver(191)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',191);">&nbsp;</span
></td><td id="191"><a href="#191">191</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_192"

 onmouseover="gutterOver(192)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',192);">&nbsp;</span
></td><td id="192"><a href="#192">192</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_193"

 onmouseover="gutterOver(193)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',193);">&nbsp;</span
></td><td id="193"><a href="#193">193</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_194"

 onmouseover="gutterOver(194)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',194);">&nbsp;</span
></td><td id="194"><a href="#194">194</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_195"

 onmouseover="gutterOver(195)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',195);">&nbsp;</span
></td><td id="195"><a href="#195">195</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_196"

 onmouseover="gutterOver(196)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',196);">&nbsp;</span
></td><td id="196"><a href="#196">196</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_197"

 onmouseover="gutterOver(197)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',197);">&nbsp;</span
></td><td id="197"><a href="#197">197</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_198"

 onmouseover="gutterOver(198)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',198);">&nbsp;</span
></td><td id="198"><a href="#198">198</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_199"

 onmouseover="gutterOver(199)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',199);">&nbsp;</span
></td><td id="199"><a href="#199">199</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_200"

 onmouseover="gutterOver(200)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',200);">&nbsp;</span
></td><td id="200"><a href="#200">200</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_201"

 onmouseover="gutterOver(201)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',201);">&nbsp;</span
></td><td id="201"><a href="#201">201</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_202"

 onmouseover="gutterOver(202)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',202);">&nbsp;</span
></td><td id="202"><a href="#202">202</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_203"

 onmouseover="gutterOver(203)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',203);">&nbsp;</span
></td><td id="203"><a href="#203">203</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_204"

 onmouseover="gutterOver(204)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',204);">&nbsp;</span
></td><td id="204"><a href="#204">204</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_205"

 onmouseover="gutterOver(205)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',205);">&nbsp;</span
></td><td id="205"><a href="#205">205</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_206"

 onmouseover="gutterOver(206)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',206);">&nbsp;</span
></td><td id="206"><a href="#206">206</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_207"

 onmouseover="gutterOver(207)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',207);">&nbsp;</span
></td><td id="207"><a href="#207">207</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_208"

 onmouseover="gutterOver(208)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',208);">&nbsp;</span
></td><td id="208"><a href="#208">208</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_209"

 onmouseover="gutterOver(209)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',209);">&nbsp;</span
></td><td id="209"><a href="#209">209</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_210"

 onmouseover="gutterOver(210)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',210);">&nbsp;</span
></td><td id="210"><a href="#210">210</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_211"

 onmouseover="gutterOver(211)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',211);">&nbsp;</span
></td><td id="211"><a href="#211">211</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_212"

 onmouseover="gutterOver(212)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',212);">&nbsp;</span
></td><td id="212"><a href="#212">212</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_213"

 onmouseover="gutterOver(213)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',213);">&nbsp;</span
></td><td id="213"><a href="#213">213</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_214"

 onmouseover="gutterOver(214)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',214);">&nbsp;</span
></td><td id="214"><a href="#214">214</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_215"

 onmouseover="gutterOver(215)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',215);">&nbsp;</span
></td><td id="215"><a href="#215">215</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_216"

 onmouseover="gutterOver(216)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',216);">&nbsp;</span
></td><td id="216"><a href="#216">216</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_217"

 onmouseover="gutterOver(217)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',217);">&nbsp;</span
></td><td id="217"><a href="#217">217</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_218"

 onmouseover="gutterOver(218)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',218);">&nbsp;</span
></td><td id="218"><a href="#218">218</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_219"

 onmouseover="gutterOver(219)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',219);">&nbsp;</span
></td><td id="219"><a href="#219">219</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_220"

 onmouseover="gutterOver(220)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',220);">&nbsp;</span
></td><td id="220"><a href="#220">220</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_221"

 onmouseover="gutterOver(221)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',221);">&nbsp;</span
></td><td id="221"><a href="#221">221</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_222"

 onmouseover="gutterOver(222)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',222);">&nbsp;</span
></td><td id="222"><a href="#222">222</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_223"

 onmouseover="gutterOver(223)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',223);">&nbsp;</span
></td><td id="223"><a href="#223">223</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_224"

 onmouseover="gutterOver(224)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',224);">&nbsp;</span
></td><td id="224"><a href="#224">224</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_225"

 onmouseover="gutterOver(225)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',225);">&nbsp;</span
></td><td id="225"><a href="#225">225</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_226"

 onmouseover="gutterOver(226)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',226);">&nbsp;</span
></td><td id="226"><a href="#226">226</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_227"

 onmouseover="gutterOver(227)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',227);">&nbsp;</span
></td><td id="227"><a href="#227">227</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_228"

 onmouseover="gutterOver(228)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',228);">&nbsp;</span
></td><td id="228"><a href="#228">228</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_229"

 onmouseover="gutterOver(229)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',229);">&nbsp;</span
></td><td id="229"><a href="#229">229</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_230"

 onmouseover="gutterOver(230)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',230);">&nbsp;</span
></td><td id="230"><a href="#230">230</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_231"

 onmouseover="gutterOver(231)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',231);">&nbsp;</span
></td><td id="231"><a href="#231">231</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_232"

 onmouseover="gutterOver(232)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',232);">&nbsp;</span
></td><td id="232"><a href="#232">232</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_233"

 onmouseover="gutterOver(233)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',233);">&nbsp;</span
></td><td id="233"><a href="#233">233</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_234"

 onmouseover="gutterOver(234)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',234);">&nbsp;</span
></td><td id="234"><a href="#234">234</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_235"

 onmouseover="gutterOver(235)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',235);">&nbsp;</span
></td><td id="235"><a href="#235">235</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_236"

 onmouseover="gutterOver(236)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',236);">&nbsp;</span
></td><td id="236"><a href="#236">236</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_237"

 onmouseover="gutterOver(237)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',237);">&nbsp;</span
></td><td id="237"><a href="#237">237</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_238"

 onmouseover="gutterOver(238)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',238);">&nbsp;</span
></td><td id="238"><a href="#238">238</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_239"

 onmouseover="gutterOver(239)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',239);">&nbsp;</span
></td><td id="239"><a href="#239">239</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_240"

 onmouseover="gutterOver(240)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',240);">&nbsp;</span
></td><td id="240"><a href="#240">240</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_241"

 onmouseover="gutterOver(241)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',241);">&nbsp;</span
></td><td id="241"><a href="#241">241</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_242"

 onmouseover="gutterOver(242)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',242);">&nbsp;</span
></td><td id="242"><a href="#242">242</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_243"

 onmouseover="gutterOver(243)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',243);">&nbsp;</span
></td><td id="243"><a href="#243">243</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_244"

 onmouseover="gutterOver(244)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',244);">&nbsp;</span
></td><td id="244"><a href="#244">244</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_245"

 onmouseover="gutterOver(245)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',245);">&nbsp;</span
></td><td id="245"><a href="#245">245</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_246"

 onmouseover="gutterOver(246)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',246);">&nbsp;</span
></td><td id="246"><a href="#246">246</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_247"

 onmouseover="gutterOver(247)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',247);">&nbsp;</span
></td><td id="247"><a href="#247">247</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_248"

 onmouseover="gutterOver(248)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',248);">&nbsp;</span
></td><td id="248"><a href="#248">248</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_249"

 onmouseover="gutterOver(249)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',249);">&nbsp;</span
></td><td id="249"><a href="#249">249</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_250"

 onmouseover="gutterOver(250)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',250);">&nbsp;</span
></td><td id="250"><a href="#250">250</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_251"

 onmouseover="gutterOver(251)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',251);">&nbsp;</span
></td><td id="251"><a href="#251">251</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_252"

 onmouseover="gutterOver(252)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',252);">&nbsp;</span
></td><td id="252"><a href="#252">252</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_253"

 onmouseover="gutterOver(253)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',253);">&nbsp;</span
></td><td id="253"><a href="#253">253</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_254"

 onmouseover="gutterOver(254)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',254);">&nbsp;</span
></td><td id="254"><a href="#254">254</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_255"

 onmouseover="gutterOver(255)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',255);">&nbsp;</span
></td><td id="255"><a href="#255">255</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_256"

 onmouseover="gutterOver(256)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',256);">&nbsp;</span
></td><td id="256"><a href="#256">256</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_257"

 onmouseover="gutterOver(257)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',257);">&nbsp;</span
></td><td id="257"><a href="#257">257</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_258"

 onmouseover="gutterOver(258)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',258);">&nbsp;</span
></td><td id="258"><a href="#258">258</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_259"

 onmouseover="gutterOver(259)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',259);">&nbsp;</span
></td><td id="259"><a href="#259">259</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_260"

 onmouseover="gutterOver(260)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',260);">&nbsp;</span
></td><td id="260"><a href="#260">260</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_261"

 onmouseover="gutterOver(261)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',261);">&nbsp;</span
></td><td id="261"><a href="#261">261</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_262"

 onmouseover="gutterOver(262)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',262);">&nbsp;</span
></td><td id="262"><a href="#262">262</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_263"

 onmouseover="gutterOver(263)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',263);">&nbsp;</span
></td><td id="263"><a href="#263">263</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_264"

 onmouseover="gutterOver(264)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',264);">&nbsp;</span
></td><td id="264"><a href="#264">264</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_265"

 onmouseover="gutterOver(265)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',265);">&nbsp;</span
></td><td id="265"><a href="#265">265</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_266"

 onmouseover="gutterOver(266)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',266);">&nbsp;</span
></td><td id="266"><a href="#266">266</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_267"

 onmouseover="gutterOver(267)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',267);">&nbsp;</span
></td><td id="267"><a href="#267">267</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_268"

 onmouseover="gutterOver(268)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',268);">&nbsp;</span
></td><td id="268"><a href="#268">268</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_269"

 onmouseover="gutterOver(269)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',269);">&nbsp;</span
></td><td id="269"><a href="#269">269</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_270"

 onmouseover="gutterOver(270)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',270);">&nbsp;</span
></td><td id="270"><a href="#270">270</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_271"

 onmouseover="gutterOver(271)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',271);">&nbsp;</span
></td><td id="271"><a href="#271">271</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_272"

 onmouseover="gutterOver(272)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',272);">&nbsp;</span
></td><td id="272"><a href="#272">272</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_273"

 onmouseover="gutterOver(273)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',273);">&nbsp;</span
></td><td id="273"><a href="#273">273</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_274"

 onmouseover="gutterOver(274)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',274);">&nbsp;</span
></td><td id="274"><a href="#274">274</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_275"

 onmouseover="gutterOver(275)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',275);">&nbsp;</span
></td><td id="275"><a href="#275">275</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_276"

 onmouseover="gutterOver(276)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',276);">&nbsp;</span
></td><td id="276"><a href="#276">276</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_277"

 onmouseover="gutterOver(277)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',277);">&nbsp;</span
></td><td id="277"><a href="#277">277</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_278"

 onmouseover="gutterOver(278)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',278);">&nbsp;</span
></td><td id="278"><a href="#278">278</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_279"

 onmouseover="gutterOver(279)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',279);">&nbsp;</span
></td><td id="279"><a href="#279">279</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_280"

 onmouseover="gutterOver(280)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',280);">&nbsp;</span
></td><td id="280"><a href="#280">280</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_281"

 onmouseover="gutterOver(281)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',281);">&nbsp;</span
></td><td id="281"><a href="#281">281</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_282"

 onmouseover="gutterOver(282)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',282);">&nbsp;</span
></td><td id="282"><a href="#282">282</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_283"

 onmouseover="gutterOver(283)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',283);">&nbsp;</span
></td><td id="283"><a href="#283">283</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_284"

 onmouseover="gutterOver(284)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',284);">&nbsp;</span
></td><td id="284"><a href="#284">284</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_285"

 onmouseover="gutterOver(285)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',285);">&nbsp;</span
></td><td id="285"><a href="#285">285</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_286"

 onmouseover="gutterOver(286)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',286);">&nbsp;</span
></td><td id="286"><a href="#286">286</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_287"

 onmouseover="gutterOver(287)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',287);">&nbsp;</span
></td><td id="287"><a href="#287">287</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_288"

 onmouseover="gutterOver(288)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',288);">&nbsp;</span
></td><td id="288"><a href="#288">288</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_289"

 onmouseover="gutterOver(289)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',289);">&nbsp;</span
></td><td id="289"><a href="#289">289</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_290"

 onmouseover="gutterOver(290)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',290);">&nbsp;</span
></td><td id="290"><a href="#290">290</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_291"

 onmouseover="gutterOver(291)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',291);">&nbsp;</span
></td><td id="291"><a href="#291">291</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_292"

 onmouseover="gutterOver(292)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',292);">&nbsp;</span
></td><td id="292"><a href="#292">292</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_293"

 onmouseover="gutterOver(293)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',293);">&nbsp;</span
></td><td id="293"><a href="#293">293</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_294"

 onmouseover="gutterOver(294)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',294);">&nbsp;</span
></td><td id="294"><a href="#294">294</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_295"

 onmouseover="gutterOver(295)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',295);">&nbsp;</span
></td><td id="295"><a href="#295">295</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_296"

 onmouseover="gutterOver(296)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',296);">&nbsp;</span
></td><td id="296"><a href="#296">296</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_297"

 onmouseover="gutterOver(297)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',297);">&nbsp;</span
></td><td id="297"><a href="#297">297</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_298"

 onmouseover="gutterOver(298)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',298);">&nbsp;</span
></td><td id="298"><a href="#298">298</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_299"

 onmouseover="gutterOver(299)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',299);">&nbsp;</span
></td><td id="299"><a href="#299">299</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_300"

 onmouseover="gutterOver(300)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',300);">&nbsp;</span
></td><td id="300"><a href="#300">300</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_301"

 onmouseover="gutterOver(301)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',301);">&nbsp;</span
></td><td id="301"><a href="#301">301</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_302"

 onmouseover="gutterOver(302)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',302);">&nbsp;</span
></td><td id="302"><a href="#302">302</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_303"

 onmouseover="gutterOver(303)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',303);">&nbsp;</span
></td><td id="303"><a href="#303">303</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_304"

 onmouseover="gutterOver(304)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',304);">&nbsp;</span
></td><td id="304"><a href="#304">304</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_305"

 onmouseover="gutterOver(305)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',305);">&nbsp;</span
></td><td id="305"><a href="#305">305</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_306"

 onmouseover="gutterOver(306)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',306);">&nbsp;</span
></td><td id="306"><a href="#306">306</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_307"

 onmouseover="gutterOver(307)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',307);">&nbsp;</span
></td><td id="307"><a href="#307">307</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_308"

 onmouseover="gutterOver(308)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',308);">&nbsp;</span
></td><td id="308"><a href="#308">308</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_309"

 onmouseover="gutterOver(309)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',309);">&nbsp;</span
></td><td id="309"><a href="#309">309</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_310"

 onmouseover="gutterOver(310)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',310);">&nbsp;</span
></td><td id="310"><a href="#310">310</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_311"

 onmouseover="gutterOver(311)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',311);">&nbsp;</span
></td><td id="311"><a href="#311">311</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_312"

 onmouseover="gutterOver(312)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',312);">&nbsp;</span
></td><td id="312"><a href="#312">312</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_313"

 onmouseover="gutterOver(313)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',313);">&nbsp;</span
></td><td id="313"><a href="#313">313</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_314"

 onmouseover="gutterOver(314)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',314);">&nbsp;</span
></td><td id="314"><a href="#314">314</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_315"

 onmouseover="gutterOver(315)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',315);">&nbsp;</span
></td><td id="315"><a href="#315">315</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_316"

 onmouseover="gutterOver(316)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',316);">&nbsp;</span
></td><td id="316"><a href="#316">316</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_317"

 onmouseover="gutterOver(317)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',317);">&nbsp;</span
></td><td id="317"><a href="#317">317</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_318"

 onmouseover="gutterOver(318)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',318);">&nbsp;</span
></td><td id="318"><a href="#318">318</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_319"

 onmouseover="gutterOver(319)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',319);">&nbsp;</span
></td><td id="319"><a href="#319">319</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_320"

 onmouseover="gutterOver(320)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',320);">&nbsp;</span
></td><td id="320"><a href="#320">320</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_321"

 onmouseover="gutterOver(321)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',321);">&nbsp;</span
></td><td id="321"><a href="#321">321</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_322"

 onmouseover="gutterOver(322)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',322);">&nbsp;</span
></td><td id="322"><a href="#322">322</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_323"

 onmouseover="gutterOver(323)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',323);">&nbsp;</span
></td><td id="323"><a href="#323">323</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_324"

 onmouseover="gutterOver(324)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',324);">&nbsp;</span
></td><td id="324"><a href="#324">324</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_325"

 onmouseover="gutterOver(325)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',325);">&nbsp;</span
></td><td id="325"><a href="#325">325</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_326"

 onmouseover="gutterOver(326)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',326);">&nbsp;</span
></td><td id="326"><a href="#326">326</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_327"

 onmouseover="gutterOver(327)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',327);">&nbsp;</span
></td><td id="327"><a href="#327">327</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_328"

 onmouseover="gutterOver(328)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',328);">&nbsp;</span
></td><td id="328"><a href="#328">328</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_329"

 onmouseover="gutterOver(329)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',329);">&nbsp;</span
></td><td id="329"><a href="#329">329</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_330"

 onmouseover="gutterOver(330)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',330);">&nbsp;</span
></td><td id="330"><a href="#330">330</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_331"

 onmouseover="gutterOver(331)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',331);">&nbsp;</span
></td><td id="331"><a href="#331">331</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_332"

 onmouseover="gutterOver(332)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',332);">&nbsp;</span
></td><td id="332"><a href="#332">332</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_333"

 onmouseover="gutterOver(333)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',333);">&nbsp;</span
></td><td id="333"><a href="#333">333</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_334"

 onmouseover="gutterOver(334)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',334);">&nbsp;</span
></td><td id="334"><a href="#334">334</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_335"

 onmouseover="gutterOver(335)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',335);">&nbsp;</span
></td><td id="335"><a href="#335">335</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_336"

 onmouseover="gutterOver(336)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',336);">&nbsp;</span
></td><td id="336"><a href="#336">336</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_337"

 onmouseover="gutterOver(337)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',337);">&nbsp;</span
></td><td id="337"><a href="#337">337</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_338"

 onmouseover="gutterOver(338)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',338);">&nbsp;</span
></td><td id="338"><a href="#338">338</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_339"

 onmouseover="gutterOver(339)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',339);">&nbsp;</span
></td><td id="339"><a href="#339">339</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_340"

 onmouseover="gutterOver(340)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',340);">&nbsp;</span
></td><td id="340"><a href="#340">340</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_341"

 onmouseover="gutterOver(341)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',341);">&nbsp;</span
></td><td id="341"><a href="#341">341</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_342"

 onmouseover="gutterOver(342)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',342);">&nbsp;</span
></td><td id="342"><a href="#342">342</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_343"

 onmouseover="gutterOver(343)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',343);">&nbsp;</span
></td><td id="343"><a href="#343">343</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_344"

 onmouseover="gutterOver(344)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',344);">&nbsp;</span
></td><td id="344"><a href="#344">344</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_345"

 onmouseover="gutterOver(345)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',345);">&nbsp;</span
></td><td id="345"><a href="#345">345</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_346"

 onmouseover="gutterOver(346)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',346);">&nbsp;</span
></td><td id="346"><a href="#346">346</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_347"

 onmouseover="gutterOver(347)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',347);">&nbsp;</span
></td><td id="347"><a href="#347">347</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_348"

 onmouseover="gutterOver(348)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',348);">&nbsp;</span
></td><td id="348"><a href="#348">348</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_349"

 onmouseover="gutterOver(349)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',349);">&nbsp;</span
></td><td id="349"><a href="#349">349</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_350"

 onmouseover="gutterOver(350)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',350);">&nbsp;</span
></td><td id="350"><a href="#350">350</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_351"

 onmouseover="gutterOver(351)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',351);">&nbsp;</span
></td><td id="351"><a href="#351">351</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_352"

 onmouseover="gutterOver(352)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',352);">&nbsp;</span
></td><td id="352"><a href="#352">352</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_353"

 onmouseover="gutterOver(353)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',353);">&nbsp;</span
></td><td id="353"><a href="#353">353</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_354"

 onmouseover="gutterOver(354)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',354);">&nbsp;</span
></td><td id="354"><a href="#354">354</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_355"

 onmouseover="gutterOver(355)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',355);">&nbsp;</span
></td><td id="355"><a href="#355">355</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_356"

 onmouseover="gutterOver(356)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',356);">&nbsp;</span
></td><td id="356"><a href="#356">356</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_357"

 onmouseover="gutterOver(357)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',357);">&nbsp;</span
></td><td id="357"><a href="#357">357</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_358"

 onmouseover="gutterOver(358)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',358);">&nbsp;</span
></td><td id="358"><a href="#358">358</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_359"

 onmouseover="gutterOver(359)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',359);">&nbsp;</span
></td><td id="359"><a href="#359">359</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_360"

 onmouseover="gutterOver(360)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',360);">&nbsp;</span
></td><td id="360"><a href="#360">360</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_361"

 onmouseover="gutterOver(361)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',361);">&nbsp;</span
></td><td id="361"><a href="#361">361</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_362"

 onmouseover="gutterOver(362)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',362);">&nbsp;</span
></td><td id="362"><a href="#362">362</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_363"

 onmouseover="gutterOver(363)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',363);">&nbsp;</span
></td><td id="363"><a href="#363">363</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_364"

 onmouseover="gutterOver(364)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',364);">&nbsp;</span
></td><td id="364"><a href="#364">364</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_365"

 onmouseover="gutterOver(365)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',365);">&nbsp;</span
></td><td id="365"><a href="#365">365</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_366"

 onmouseover="gutterOver(366)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',366);">&nbsp;</span
></td><td id="366"><a href="#366">366</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_367"

 onmouseover="gutterOver(367)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',367);">&nbsp;</span
></td><td id="367"><a href="#367">367</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_368"

 onmouseover="gutterOver(368)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',368);">&nbsp;</span
></td><td id="368"><a href="#368">368</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_369"

 onmouseover="gutterOver(369)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',369);">&nbsp;</span
></td><td id="369"><a href="#369">369</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_370"

 onmouseover="gutterOver(370)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',370);">&nbsp;</span
></td><td id="370"><a href="#370">370</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_371"

 onmouseover="gutterOver(371)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',371);">&nbsp;</span
></td><td id="371"><a href="#371">371</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_372"

 onmouseover="gutterOver(372)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',372);">&nbsp;</span
></td><td id="372"><a href="#372">372</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_373"

 onmouseover="gutterOver(373)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',373);">&nbsp;</span
></td><td id="373"><a href="#373">373</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_374"

 onmouseover="gutterOver(374)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',374);">&nbsp;</span
></td><td id="374"><a href="#374">374</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_375"

 onmouseover="gutterOver(375)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',375);">&nbsp;</span
></td><td id="375"><a href="#375">375</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_376"

 onmouseover="gutterOver(376)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',376);">&nbsp;</span
></td><td id="376"><a href="#376">376</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_377"

 onmouseover="gutterOver(377)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',377);">&nbsp;</span
></td><td id="377"><a href="#377">377</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_378"

 onmouseover="gutterOver(378)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',378);">&nbsp;</span
></td><td id="378"><a href="#378">378</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_379"

 onmouseover="gutterOver(379)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',379);">&nbsp;</span
></td><td id="379"><a href="#379">379</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_380"

 onmouseover="gutterOver(380)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',380);">&nbsp;</span
></td><td id="380"><a href="#380">380</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_381"

 onmouseover="gutterOver(381)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',381);">&nbsp;</span
></td><td id="381"><a href="#381">381</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_382"

 onmouseover="gutterOver(382)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',382);">&nbsp;</span
></td><td id="382"><a href="#382">382</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_383"

 onmouseover="gutterOver(383)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',383);">&nbsp;</span
></td><td id="383"><a href="#383">383</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_384"

 onmouseover="gutterOver(384)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',384);">&nbsp;</span
></td><td id="384"><a href="#384">384</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_385"

 onmouseover="gutterOver(385)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',385);">&nbsp;</span
></td><td id="385"><a href="#385">385</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_386"

 onmouseover="gutterOver(386)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',386);">&nbsp;</span
></td><td id="386"><a href="#386">386</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_387"

 onmouseover="gutterOver(387)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',387);">&nbsp;</span
></td><td id="387"><a href="#387">387</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_388"

 onmouseover="gutterOver(388)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',388);">&nbsp;</span
></td><td id="388"><a href="#388">388</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_389"

 onmouseover="gutterOver(389)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',389);">&nbsp;</span
></td><td id="389"><a href="#389">389</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_390"

 onmouseover="gutterOver(390)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',390);">&nbsp;</span
></td><td id="390"><a href="#390">390</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_391"

 onmouseover="gutterOver(391)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',391);">&nbsp;</span
></td><td id="391"><a href="#391">391</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_392"

 onmouseover="gutterOver(392)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',392);">&nbsp;</span
></td><td id="392"><a href="#392">392</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_393"

 onmouseover="gutterOver(393)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',393);">&nbsp;</span
></td><td id="393"><a href="#393">393</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_394"

 onmouseover="gutterOver(394)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',394);">&nbsp;</span
></td><td id="394"><a href="#394">394</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_395"

 onmouseover="gutterOver(395)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',395);">&nbsp;</span
></td><td id="395"><a href="#395">395</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_396"

 onmouseover="gutterOver(396)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',396);">&nbsp;</span
></td><td id="396"><a href="#396">396</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_397"

 onmouseover="gutterOver(397)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',397);">&nbsp;</span
></td><td id="397"><a href="#397">397</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_398"

 onmouseover="gutterOver(398)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',398);">&nbsp;</span
></td><td id="398"><a href="#398">398</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_399"

 onmouseover="gutterOver(399)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',399);">&nbsp;</span
></td><td id="399"><a href="#399">399</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_400"

 onmouseover="gutterOver(400)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',400);">&nbsp;</span
></td><td id="400"><a href="#400">400</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_401"

 onmouseover="gutterOver(401)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',401);">&nbsp;</span
></td><td id="401"><a href="#401">401</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_402"

 onmouseover="gutterOver(402)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',402);">&nbsp;</span
></td><td id="402"><a href="#402">402</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_403"

 onmouseover="gutterOver(403)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',403);">&nbsp;</span
></td><td id="403"><a href="#403">403</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_404"

 onmouseover="gutterOver(404)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',404);">&nbsp;</span
></td><td id="404"><a href="#404">404</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_405"

 onmouseover="gutterOver(405)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',405);">&nbsp;</span
></td><td id="405"><a href="#405">405</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_406"

 onmouseover="gutterOver(406)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',406);">&nbsp;</span
></td><td id="406"><a href="#406">406</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_407"

 onmouseover="gutterOver(407)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',407);">&nbsp;</span
></td><td id="407"><a href="#407">407</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_408"

 onmouseover="gutterOver(408)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',408);">&nbsp;</span
></td><td id="408"><a href="#408">408</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_409"

 onmouseover="gutterOver(409)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',409);">&nbsp;</span
></td><td id="409"><a href="#409">409</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_410"

 onmouseover="gutterOver(410)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',410);">&nbsp;</span
></td><td id="410"><a href="#410">410</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_411"

 onmouseover="gutterOver(411)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',411);">&nbsp;</span
></td><td id="411"><a href="#411">411</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_412"

 onmouseover="gutterOver(412)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',412);">&nbsp;</span
></td><td id="412"><a href="#412">412</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_413"

 onmouseover="gutterOver(413)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',413);">&nbsp;</span
></td><td id="413"><a href="#413">413</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_414"

 onmouseover="gutterOver(414)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',414);">&nbsp;</span
></td><td id="414"><a href="#414">414</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_415"

 onmouseover="gutterOver(415)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',415);">&nbsp;</span
></td><td id="415"><a href="#415">415</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_416"

 onmouseover="gutterOver(416)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',416);">&nbsp;</span
></td><td id="416"><a href="#416">416</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_417"

 onmouseover="gutterOver(417)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',417);">&nbsp;</span
></td><td id="417"><a href="#417">417</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_418"

 onmouseover="gutterOver(418)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',418);">&nbsp;</span
></td><td id="418"><a href="#418">418</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_419"

 onmouseover="gutterOver(419)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',419);">&nbsp;</span
></td><td id="419"><a href="#419">419</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_420"

 onmouseover="gutterOver(420)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',420);">&nbsp;</span
></td><td id="420"><a href="#420">420</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_421"

 onmouseover="gutterOver(421)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',421);">&nbsp;</span
></td><td id="421"><a href="#421">421</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_422"

 onmouseover="gutterOver(422)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',422);">&nbsp;</span
></td><td id="422"><a href="#422">422</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_423"

 onmouseover="gutterOver(423)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',423);">&nbsp;</span
></td><td id="423"><a href="#423">423</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_424"

 onmouseover="gutterOver(424)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',424);">&nbsp;</span
></td><td id="424"><a href="#424">424</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_425"

 onmouseover="gutterOver(425)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',425);">&nbsp;</span
></td><td id="425"><a href="#425">425</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_426"

 onmouseover="gutterOver(426)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',426);">&nbsp;</span
></td><td id="426"><a href="#426">426</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_427"

 onmouseover="gutterOver(427)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',427);">&nbsp;</span
></td><td id="427"><a href="#427">427</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_428"

 onmouseover="gutterOver(428)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',428);">&nbsp;</span
></td><td id="428"><a href="#428">428</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_429"

 onmouseover="gutterOver(429)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',429);">&nbsp;</span
></td><td id="429"><a href="#429">429</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_430"

 onmouseover="gutterOver(430)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',430);">&nbsp;</span
></td><td id="430"><a href="#430">430</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_431"

 onmouseover="gutterOver(431)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',431);">&nbsp;</span
></td><td id="431"><a href="#431">431</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_432"

 onmouseover="gutterOver(432)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',432);">&nbsp;</span
></td><td id="432"><a href="#432">432</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_433"

 onmouseover="gutterOver(433)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',433);">&nbsp;</span
></td><td id="433"><a href="#433">433</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_434"

 onmouseover="gutterOver(434)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',434);">&nbsp;</span
></td><td id="434"><a href="#434">434</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_435"

 onmouseover="gutterOver(435)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',435);">&nbsp;</span
></td><td id="435"><a href="#435">435</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_436"

 onmouseover="gutterOver(436)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',436);">&nbsp;</span
></td><td id="436"><a href="#436">436</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_437"

 onmouseover="gutterOver(437)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',437);">&nbsp;</span
></td><td id="437"><a href="#437">437</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_438"

 onmouseover="gutterOver(438)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',438);">&nbsp;</span
></td><td id="438"><a href="#438">438</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_439"

 onmouseover="gutterOver(439)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',439);">&nbsp;</span
></td><td id="439"><a href="#439">439</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_440"

 onmouseover="gutterOver(440)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',440);">&nbsp;</span
></td><td id="440"><a href="#440">440</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_441"

 onmouseover="gutterOver(441)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',441);">&nbsp;</span
></td><td id="441"><a href="#441">441</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_442"

 onmouseover="gutterOver(442)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',442);">&nbsp;</span
></td><td id="442"><a href="#442">442</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_443"

 onmouseover="gutterOver(443)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',443);">&nbsp;</span
></td><td id="443"><a href="#443">443</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_444"

 onmouseover="gutterOver(444)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',444);">&nbsp;</span
></td><td id="444"><a href="#444">444</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_445"

 onmouseover="gutterOver(445)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',445);">&nbsp;</span
></td><td id="445"><a href="#445">445</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_446"

 onmouseover="gutterOver(446)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',446);">&nbsp;</span
></td><td id="446"><a href="#446">446</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_447"

 onmouseover="gutterOver(447)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',447);">&nbsp;</span
></td><td id="447"><a href="#447">447</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_448"

 onmouseover="gutterOver(448)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',448);">&nbsp;</span
></td><td id="448"><a href="#448">448</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_449"

 onmouseover="gutterOver(449)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',449);">&nbsp;</span
></td><td id="449"><a href="#449">449</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_450"

 onmouseover="gutterOver(450)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',450);">&nbsp;</span
></td><td id="450"><a href="#450">450</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_451"

 onmouseover="gutterOver(451)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',451);">&nbsp;</span
></td><td id="451"><a href="#451">451</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_452"

 onmouseover="gutterOver(452)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',452);">&nbsp;</span
></td><td id="452"><a href="#452">452</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_453"

 onmouseover="gutterOver(453)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',453);">&nbsp;</span
></td><td id="453"><a href="#453">453</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_454"

 onmouseover="gutterOver(454)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',454);">&nbsp;</span
></td><td id="454"><a href="#454">454</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_455"

 onmouseover="gutterOver(455)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',455);">&nbsp;</span
></td><td id="455"><a href="#455">455</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_456"

 onmouseover="gutterOver(456)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',456);">&nbsp;</span
></td><td id="456"><a href="#456">456</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_457"

 onmouseover="gutterOver(457)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',457);">&nbsp;</span
></td><td id="457"><a href="#457">457</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_458"

 onmouseover="gutterOver(458)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',458);">&nbsp;</span
></td><td id="458"><a href="#458">458</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_459"

 onmouseover="gutterOver(459)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',459);">&nbsp;</span
></td><td id="459"><a href="#459">459</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_460"

 onmouseover="gutterOver(460)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',460);">&nbsp;</span
></td><td id="460"><a href="#460">460</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_461"

 onmouseover="gutterOver(461)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',461);">&nbsp;</span
></td><td id="461"><a href="#461">461</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_462"

 onmouseover="gutterOver(462)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',462);">&nbsp;</span
></td><td id="462"><a href="#462">462</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_463"

 onmouseover="gutterOver(463)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',463);">&nbsp;</span
></td><td id="463"><a href="#463">463</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_464"

 onmouseover="gutterOver(464)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',464);">&nbsp;</span
></td><td id="464"><a href="#464">464</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_465"

 onmouseover="gutterOver(465)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',465);">&nbsp;</span
></td><td id="465"><a href="#465">465</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_466"

 onmouseover="gutterOver(466)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',466);">&nbsp;</span
></td><td id="466"><a href="#466">466</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_467"

 onmouseover="gutterOver(467)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',467);">&nbsp;</span
></td><td id="467"><a href="#467">467</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_468"

 onmouseover="gutterOver(468)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',468);">&nbsp;</span
></td><td id="468"><a href="#468">468</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_469"

 onmouseover="gutterOver(469)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',469);">&nbsp;</span
></td><td id="469"><a href="#469">469</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_470"

 onmouseover="gutterOver(470)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',470);">&nbsp;</span
></td><td id="470"><a href="#470">470</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_471"

 onmouseover="gutterOver(471)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',471);">&nbsp;</span
></td><td id="471"><a href="#471">471</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_472"

 onmouseover="gutterOver(472)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',472);">&nbsp;</span
></td><td id="472"><a href="#472">472</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_473"

 onmouseover="gutterOver(473)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',473);">&nbsp;</span
></td><td id="473"><a href="#473">473</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_474"

 onmouseover="gutterOver(474)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',474);">&nbsp;</span
></td><td id="474"><a href="#474">474</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_475"

 onmouseover="gutterOver(475)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',475);">&nbsp;</span
></td><td id="475"><a href="#475">475</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_476"

 onmouseover="gutterOver(476)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',476);">&nbsp;</span
></td><td id="476"><a href="#476">476</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_477"

 onmouseover="gutterOver(477)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',477);">&nbsp;</span
></td><td id="477"><a href="#477">477</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_478"

 onmouseover="gutterOver(478)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',478);">&nbsp;</span
></td><td id="478"><a href="#478">478</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_479"

 onmouseover="gutterOver(479)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',479);">&nbsp;</span
></td><td id="479"><a href="#479">479</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_480"

 onmouseover="gutterOver(480)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',480);">&nbsp;</span
></td><td id="480"><a href="#480">480</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_481"

 onmouseover="gutterOver(481)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',481);">&nbsp;</span
></td><td id="481"><a href="#481">481</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_482"

 onmouseover="gutterOver(482)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',482);">&nbsp;</span
></td><td id="482"><a href="#482">482</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_483"

 onmouseover="gutterOver(483)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',483);">&nbsp;</span
></td><td id="483"><a href="#483">483</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_484"

 onmouseover="gutterOver(484)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',484);">&nbsp;</span
></td><td id="484"><a href="#484">484</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_485"

 onmouseover="gutterOver(485)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',485);">&nbsp;</span
></td><td id="485"><a href="#485">485</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_486"

 onmouseover="gutterOver(486)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',486);">&nbsp;</span
></td><td id="486"><a href="#486">486</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_487"

 onmouseover="gutterOver(487)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',487);">&nbsp;</span
></td><td id="487"><a href="#487">487</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_488"

 onmouseover="gutterOver(488)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',488);">&nbsp;</span
></td><td id="488"><a href="#488">488</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_489"

 onmouseover="gutterOver(489)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',489);">&nbsp;</span
></td><td id="489"><a href="#489">489</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_490"

 onmouseover="gutterOver(490)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',490);">&nbsp;</span
></td><td id="490"><a href="#490">490</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_491"

 onmouseover="gutterOver(491)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',491);">&nbsp;</span
></td><td id="491"><a href="#491">491</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_492"

 onmouseover="gutterOver(492)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',492);">&nbsp;</span
></td><td id="492"><a href="#492">492</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_493"

 onmouseover="gutterOver(493)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',493);">&nbsp;</span
></td><td id="493"><a href="#493">493</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_494"

 onmouseover="gutterOver(494)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',494);">&nbsp;</span
></td><td id="494"><a href="#494">494</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_495"

 onmouseover="gutterOver(495)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',495);">&nbsp;</span
></td><td id="495"><a href="#495">495</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_496"

 onmouseover="gutterOver(496)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',496);">&nbsp;</span
></td><td id="496"><a href="#496">496</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_497"

 onmouseover="gutterOver(497)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',497);">&nbsp;</span
></td><td id="497"><a href="#497">497</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_498"

 onmouseover="gutterOver(498)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',498);">&nbsp;</span
></td><td id="498"><a href="#498">498</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_499"

 onmouseover="gutterOver(499)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',499);">&nbsp;</span
></td><td id="499"><a href="#499">499</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_500"

 onmouseover="gutterOver(500)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',500);">&nbsp;</span
></td><td id="500"><a href="#500">500</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_501"

 onmouseover="gutterOver(501)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',501);">&nbsp;</span
></td><td id="501"><a href="#501">501</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_502"

 onmouseover="gutterOver(502)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',502);">&nbsp;</span
></td><td id="502"><a href="#502">502</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_503"

 onmouseover="gutterOver(503)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',503);">&nbsp;</span
></td><td id="503"><a href="#503">503</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_504"

 onmouseover="gutterOver(504)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',504);">&nbsp;</span
></td><td id="504"><a href="#504">504</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_505"

 onmouseover="gutterOver(505)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',505);">&nbsp;</span
></td><td id="505"><a href="#505">505</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_506"

 onmouseover="gutterOver(506)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',506);">&nbsp;</span
></td><td id="506"><a href="#506">506</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_507"

 onmouseover="gutterOver(507)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',507);">&nbsp;</span
></td><td id="507"><a href="#507">507</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_508"

 onmouseover="gutterOver(508)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',508);">&nbsp;</span
></td><td id="508"><a href="#508">508</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_509"

 onmouseover="gutterOver(509)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',509);">&nbsp;</span
></td><td id="509"><a href="#509">509</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_510"

 onmouseover="gutterOver(510)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',510);">&nbsp;</span
></td><td id="510"><a href="#510">510</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_511"

 onmouseover="gutterOver(511)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',511);">&nbsp;</span
></td><td id="511"><a href="#511">511</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_512"

 onmouseover="gutterOver(512)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',512);">&nbsp;</span
></td><td id="512"><a href="#512">512</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_513"

 onmouseover="gutterOver(513)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',513);">&nbsp;</span
></td><td id="513"><a href="#513">513</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_514"

 onmouseover="gutterOver(514)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',514);">&nbsp;</span
></td><td id="514"><a href="#514">514</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_515"

 onmouseover="gutterOver(515)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',515);">&nbsp;</span
></td><td id="515"><a href="#515">515</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_516"

 onmouseover="gutterOver(516)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',516);">&nbsp;</span
></td><td id="516"><a href="#516">516</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_517"

 onmouseover="gutterOver(517)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',517);">&nbsp;</span
></td><td id="517"><a href="#517">517</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_518"

 onmouseover="gutterOver(518)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',518);">&nbsp;</span
></td><td id="518"><a href="#518">518</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_519"

 onmouseover="gutterOver(519)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',519);">&nbsp;</span
></td><td id="519"><a href="#519">519</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_520"

 onmouseover="gutterOver(520)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',520);">&nbsp;</span
></td><td id="520"><a href="#520">520</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_521"

 onmouseover="gutterOver(521)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',521);">&nbsp;</span
></td><td id="521"><a href="#521">521</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_522"

 onmouseover="gutterOver(522)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',522);">&nbsp;</span
></td><td id="522"><a href="#522">522</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_523"

 onmouseover="gutterOver(523)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',523);">&nbsp;</span
></td><td id="523"><a href="#523">523</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_524"

 onmouseover="gutterOver(524)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',524);">&nbsp;</span
></td><td id="524"><a href="#524">524</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_525"

 onmouseover="gutterOver(525)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',525);">&nbsp;</span
></td><td id="525"><a href="#525">525</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_526"

 onmouseover="gutterOver(526)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',526);">&nbsp;</span
></td><td id="526"><a href="#526">526</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_527"

 onmouseover="gutterOver(527)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',527);">&nbsp;</span
></td><td id="527"><a href="#527">527</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_528"

 onmouseover="gutterOver(528)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',528);">&nbsp;</span
></td><td id="528"><a href="#528">528</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_529"

 onmouseover="gutterOver(529)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',529);">&nbsp;</span
></td><td id="529"><a href="#529">529</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_530"

 onmouseover="gutterOver(530)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',530);">&nbsp;</span
></td><td id="530"><a href="#530">530</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_531"

 onmouseover="gutterOver(531)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',531);">&nbsp;</span
></td><td id="531"><a href="#531">531</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_532"

 onmouseover="gutterOver(532)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',532);">&nbsp;</span
></td><td id="532"><a href="#532">532</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_533"

 onmouseover="gutterOver(533)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',533);">&nbsp;</span
></td><td id="533"><a href="#533">533</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_534"

 onmouseover="gutterOver(534)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',534);">&nbsp;</span
></td><td id="534"><a href="#534">534</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_535"

 onmouseover="gutterOver(535)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',535);">&nbsp;</span
></td><td id="535"><a href="#535">535</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_536"

 onmouseover="gutterOver(536)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',536);">&nbsp;</span
></td><td id="536"><a href="#536">536</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_537"

 onmouseover="gutterOver(537)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',537);">&nbsp;</span
></td><td id="537"><a href="#537">537</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_538"

 onmouseover="gutterOver(538)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',538);">&nbsp;</span
></td><td id="538"><a href="#538">538</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_539"

 onmouseover="gutterOver(539)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',539);">&nbsp;</span
></td><td id="539"><a href="#539">539</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_540"

 onmouseover="gutterOver(540)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',540);">&nbsp;</span
></td><td id="540"><a href="#540">540</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_541"

 onmouseover="gutterOver(541)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',541);">&nbsp;</span
></td><td id="541"><a href="#541">541</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_542"

 onmouseover="gutterOver(542)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',542);">&nbsp;</span
></td><td id="542"><a href="#542">542</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_543"

 onmouseover="gutterOver(543)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',543);">&nbsp;</span
></td><td id="543"><a href="#543">543</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_544"

 onmouseover="gutterOver(544)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',544);">&nbsp;</span
></td><td id="544"><a href="#544">544</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_545"

 onmouseover="gutterOver(545)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',545);">&nbsp;</span
></td><td id="545"><a href="#545">545</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_546"

 onmouseover="gutterOver(546)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',546);">&nbsp;</span
></td><td id="546"><a href="#546">546</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_547"

 onmouseover="gutterOver(547)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',547);">&nbsp;</span
></td><td id="547"><a href="#547">547</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_548"

 onmouseover="gutterOver(548)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',548);">&nbsp;</span
></td><td id="548"><a href="#548">548</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_549"

 onmouseover="gutterOver(549)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',549);">&nbsp;</span
></td><td id="549"><a href="#549">549</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_550"

 onmouseover="gutterOver(550)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',550);">&nbsp;</span
></td><td id="550"><a href="#550">550</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_551"

 onmouseover="gutterOver(551)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',551);">&nbsp;</span
></td><td id="551"><a href="#551">551</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_552"

 onmouseover="gutterOver(552)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',552);">&nbsp;</span
></td><td id="552"><a href="#552">552</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_553"

 onmouseover="gutterOver(553)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',553);">&nbsp;</span
></td><td id="553"><a href="#553">553</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_554"

 onmouseover="gutterOver(554)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',554);">&nbsp;</span
></td><td id="554"><a href="#554">554</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_555"

 onmouseover="gutterOver(555)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',555);">&nbsp;</span
></td><td id="555"><a href="#555">555</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_556"

 onmouseover="gutterOver(556)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',556);">&nbsp;</span
></td><td id="556"><a href="#556">556</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_557"

 onmouseover="gutterOver(557)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',557);">&nbsp;</span
></td><td id="557"><a href="#557">557</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_558"

 onmouseover="gutterOver(558)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',558);">&nbsp;</span
></td><td id="558"><a href="#558">558</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_559"

 onmouseover="gutterOver(559)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',559);">&nbsp;</span
></td><td id="559"><a href="#559">559</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_560"

 onmouseover="gutterOver(560)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',560);">&nbsp;</span
></td><td id="560"><a href="#560">560</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_561"

 onmouseover="gutterOver(561)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',561);">&nbsp;</span
></td><td id="561"><a href="#561">561</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_562"

 onmouseover="gutterOver(562)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',562);">&nbsp;</span
></td><td id="562"><a href="#562">562</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_563"

 onmouseover="gutterOver(563)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',563);">&nbsp;</span
></td><td id="563"><a href="#563">563</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_564"

 onmouseover="gutterOver(564)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',564);">&nbsp;</span
></td><td id="564"><a href="#564">564</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_565"

 onmouseover="gutterOver(565)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',565);">&nbsp;</span
></td><td id="565"><a href="#565">565</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_566"

 onmouseover="gutterOver(566)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',566);">&nbsp;</span
></td><td id="566"><a href="#566">566</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_567"

 onmouseover="gutterOver(567)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',567);">&nbsp;</span
></td><td id="567"><a href="#567">567</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_568"

 onmouseover="gutterOver(568)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',568);">&nbsp;</span
></td><td id="568"><a href="#568">568</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_569"

 onmouseover="gutterOver(569)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',569);">&nbsp;</span
></td><td id="569"><a href="#569">569</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_570"

 onmouseover="gutterOver(570)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',570);">&nbsp;</span
></td><td id="570"><a href="#570">570</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_571"

 onmouseover="gutterOver(571)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',571);">&nbsp;</span
></td><td id="571"><a href="#571">571</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_572"

 onmouseover="gutterOver(572)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',572);">&nbsp;</span
></td><td id="572"><a href="#572">572</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_573"

 onmouseover="gutterOver(573)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',573);">&nbsp;</span
></td><td id="573"><a href="#573">573</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_574"

 onmouseover="gutterOver(574)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',574);">&nbsp;</span
></td><td id="574"><a href="#574">574</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_575"

 onmouseover="gutterOver(575)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',575);">&nbsp;</span
></td><td id="575"><a href="#575">575</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_576"

 onmouseover="gutterOver(576)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',576);">&nbsp;</span
></td><td id="576"><a href="#576">576</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_577"

 onmouseover="gutterOver(577)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',577);">&nbsp;</span
></td><td id="577"><a href="#577">577</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_578"

 onmouseover="gutterOver(578)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',578);">&nbsp;</span
></td><td id="578"><a href="#578">578</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_579"

 onmouseover="gutterOver(579)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',579);">&nbsp;</span
></td><td id="579"><a href="#579">579</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_580"

 onmouseover="gutterOver(580)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',580);">&nbsp;</span
></td><td id="580"><a href="#580">580</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_581"

 onmouseover="gutterOver(581)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',581);">&nbsp;</span
></td><td id="581"><a href="#581">581</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_582"

 onmouseover="gutterOver(582)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',582);">&nbsp;</span
></td><td id="582"><a href="#582">582</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_583"

 onmouseover="gutterOver(583)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',583);">&nbsp;</span
></td><td id="583"><a href="#583">583</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_584"

 onmouseover="gutterOver(584)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',584);">&nbsp;</span
></td><td id="584"><a href="#584">584</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_585"

 onmouseover="gutterOver(585)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',585);">&nbsp;</span
></td><td id="585"><a href="#585">585</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_586"

 onmouseover="gutterOver(586)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',586);">&nbsp;</span
></td><td id="586"><a href="#586">586</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_587"

 onmouseover="gutterOver(587)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',587);">&nbsp;</span
></td><td id="587"><a href="#587">587</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_588"

 onmouseover="gutterOver(588)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',588);">&nbsp;</span
></td><td id="588"><a href="#588">588</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_589"

 onmouseover="gutterOver(589)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',589);">&nbsp;</span
></td><td id="589"><a href="#589">589</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_590"

 onmouseover="gutterOver(590)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',590);">&nbsp;</span
></td><td id="590"><a href="#590">590</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_591"

 onmouseover="gutterOver(591)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',591);">&nbsp;</span
></td><td id="591"><a href="#591">591</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_592"

 onmouseover="gutterOver(592)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',592);">&nbsp;</span
></td><td id="592"><a href="#592">592</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_593"

 onmouseover="gutterOver(593)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',593);">&nbsp;</span
></td><td id="593"><a href="#593">593</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_594"

 onmouseover="gutterOver(594)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',594);">&nbsp;</span
></td><td id="594"><a href="#594">594</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_595"

 onmouseover="gutterOver(595)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',595);">&nbsp;</span
></td><td id="595"><a href="#595">595</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_596"

 onmouseover="gutterOver(596)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',596);">&nbsp;</span
></td><td id="596"><a href="#596">596</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_597"

 onmouseover="gutterOver(597)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',597);">&nbsp;</span
></td><td id="597"><a href="#597">597</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_598"

 onmouseover="gutterOver(598)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',598);">&nbsp;</span
></td><td id="598"><a href="#598">598</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_599"

 onmouseover="gutterOver(599)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',599);">&nbsp;</span
></td><td id="599"><a href="#599">599</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_600"

 onmouseover="gutterOver(600)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',600);">&nbsp;</span
></td><td id="600"><a href="#600">600</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_601"

 onmouseover="gutterOver(601)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',601);">&nbsp;</span
></td><td id="601"><a href="#601">601</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_602"

 onmouseover="gutterOver(602)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',602);">&nbsp;</span
></td><td id="602"><a href="#602">602</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_603"

 onmouseover="gutterOver(603)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',603);">&nbsp;</span
></td><td id="603"><a href="#603">603</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_604"

 onmouseover="gutterOver(604)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',604);">&nbsp;</span
></td><td id="604"><a href="#604">604</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_605"

 onmouseover="gutterOver(605)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',605);">&nbsp;</span
></td><td id="605"><a href="#605">605</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_606"

 onmouseover="gutterOver(606)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',606);">&nbsp;</span
></td><td id="606"><a href="#606">606</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_607"

 onmouseover="gutterOver(607)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',607);">&nbsp;</span
></td><td id="607"><a href="#607">607</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_608"

 onmouseover="gutterOver(608)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',608);">&nbsp;</span
></td><td id="608"><a href="#608">608</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_609"

 onmouseover="gutterOver(609)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',609);">&nbsp;</span
></td><td id="609"><a href="#609">609</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_610"

 onmouseover="gutterOver(610)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',610);">&nbsp;</span
></td><td id="610"><a href="#610">610</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_611"

 onmouseover="gutterOver(611)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',611);">&nbsp;</span
></td><td id="611"><a href="#611">611</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_612"

 onmouseover="gutterOver(612)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',612);">&nbsp;</span
></td><td id="612"><a href="#612">612</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_613"

 onmouseover="gutterOver(613)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',613);">&nbsp;</span
></td><td id="613"><a href="#613">613</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_614"

 onmouseover="gutterOver(614)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',614);">&nbsp;</span
></td><td id="614"><a href="#614">614</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_615"

 onmouseover="gutterOver(615)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',615);">&nbsp;</span
></td><td id="615"><a href="#615">615</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_616"

 onmouseover="gutterOver(616)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',616);">&nbsp;</span
></td><td id="616"><a href="#616">616</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_617"

 onmouseover="gutterOver(617)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',617);">&nbsp;</span
></td><td id="617"><a href="#617">617</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_618"

 onmouseover="gutterOver(618)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',618);">&nbsp;</span
></td><td id="618"><a href="#618">618</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_619"

 onmouseover="gutterOver(619)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',619);">&nbsp;</span
></td><td id="619"><a href="#619">619</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_620"

 onmouseover="gutterOver(620)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',620);">&nbsp;</span
></td><td id="620"><a href="#620">620</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_621"

 onmouseover="gutterOver(621)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',621);">&nbsp;</span
></td><td id="621"><a href="#621">621</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_622"

 onmouseover="gutterOver(622)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',622);">&nbsp;</span
></td><td id="622"><a href="#622">622</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_623"

 onmouseover="gutterOver(623)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',623);">&nbsp;</span
></td><td id="623"><a href="#623">623</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_624"

 onmouseover="gutterOver(624)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',624);">&nbsp;</span
></td><td id="624"><a href="#624">624</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_625"

 onmouseover="gutterOver(625)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',625);">&nbsp;</span
></td><td id="625"><a href="#625">625</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_626"

 onmouseover="gutterOver(626)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',626);">&nbsp;</span
></td><td id="626"><a href="#626">626</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_627"

 onmouseover="gutterOver(627)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',627);">&nbsp;</span
></td><td id="627"><a href="#627">627</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_628"

 onmouseover="gutterOver(628)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',628);">&nbsp;</span
></td><td id="628"><a href="#628">628</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_629"

 onmouseover="gutterOver(629)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',629);">&nbsp;</span
></td><td id="629"><a href="#629">629</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_630"

 onmouseover="gutterOver(630)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',630);">&nbsp;</span
></td><td id="630"><a href="#630">630</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_631"

 onmouseover="gutterOver(631)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',631);">&nbsp;</span
></td><td id="631"><a href="#631">631</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_632"

 onmouseover="gutterOver(632)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',632);">&nbsp;</span
></td><td id="632"><a href="#632">632</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_633"

 onmouseover="gutterOver(633)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',633);">&nbsp;</span
></td><td id="633"><a href="#633">633</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_634"

 onmouseover="gutterOver(634)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',634);">&nbsp;</span
></td><td id="634"><a href="#634">634</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_635"

 onmouseover="gutterOver(635)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',635);">&nbsp;</span
></td><td id="635"><a href="#635">635</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_636"

 onmouseover="gutterOver(636)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',636);">&nbsp;</span
></td><td id="636"><a href="#636">636</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_637"

 onmouseover="gutterOver(637)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',637);">&nbsp;</span
></td><td id="637"><a href="#637">637</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_638"

 onmouseover="gutterOver(638)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',638);">&nbsp;</span
></td><td id="638"><a href="#638">638</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_639"

 onmouseover="gutterOver(639)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',639);">&nbsp;</span
></td><td id="639"><a href="#639">639</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_640"

 onmouseover="gutterOver(640)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',640);">&nbsp;</span
></td><td id="640"><a href="#640">640</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_641"

 onmouseover="gutterOver(641)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',641);">&nbsp;</span
></td><td id="641"><a href="#641">641</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_642"

 onmouseover="gutterOver(642)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',642);">&nbsp;</span
></td><td id="642"><a href="#642">642</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_643"

 onmouseover="gutterOver(643)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',643);">&nbsp;</span
></td><td id="643"><a href="#643">643</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_644"

 onmouseover="gutterOver(644)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',644);">&nbsp;</span
></td><td id="644"><a href="#644">644</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_645"

 onmouseover="gutterOver(645)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',645);">&nbsp;</span
></td><td id="645"><a href="#645">645</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_646"

 onmouseover="gutterOver(646)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',646);">&nbsp;</span
></td><td id="646"><a href="#646">646</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_647"

 onmouseover="gutterOver(647)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',647);">&nbsp;</span
></td><td id="647"><a href="#647">647</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_648"

 onmouseover="gutterOver(648)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',648);">&nbsp;</span
></td><td id="648"><a href="#648">648</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_649"

 onmouseover="gutterOver(649)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',649);">&nbsp;</span
></td><td id="649"><a href="#649">649</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_650"

 onmouseover="gutterOver(650)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',650);">&nbsp;</span
></td><td id="650"><a href="#650">650</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_651"

 onmouseover="gutterOver(651)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',651);">&nbsp;</span
></td><td id="651"><a href="#651">651</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_652"

 onmouseover="gutterOver(652)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',652);">&nbsp;</span
></td><td id="652"><a href="#652">652</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_653"

 onmouseover="gutterOver(653)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',653);">&nbsp;</span
></td><td id="653"><a href="#653">653</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_654"

 onmouseover="gutterOver(654)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',654);">&nbsp;</span
></td><td id="654"><a href="#654">654</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_655"

 onmouseover="gutterOver(655)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',655);">&nbsp;</span
></td><td id="655"><a href="#655">655</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_656"

 onmouseover="gutterOver(656)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',656);">&nbsp;</span
></td><td id="656"><a href="#656">656</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_657"

 onmouseover="gutterOver(657)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',657);">&nbsp;</span
></td><td id="657"><a href="#657">657</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_658"

 onmouseover="gutterOver(658)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',658);">&nbsp;</span
></td><td id="658"><a href="#658">658</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_659"

 onmouseover="gutterOver(659)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',659);">&nbsp;</span
></td><td id="659"><a href="#659">659</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_660"

 onmouseover="gutterOver(660)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',660);">&nbsp;</span
></td><td id="660"><a href="#660">660</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_661"

 onmouseover="gutterOver(661)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',661);">&nbsp;</span
></td><td id="661"><a href="#661">661</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_662"

 onmouseover="gutterOver(662)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',662);">&nbsp;</span
></td><td id="662"><a href="#662">662</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_663"

 onmouseover="gutterOver(663)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',663);">&nbsp;</span
></td><td id="663"><a href="#663">663</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_664"

 onmouseover="gutterOver(664)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',664);">&nbsp;</span
></td><td id="664"><a href="#664">664</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_665"

 onmouseover="gutterOver(665)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',665);">&nbsp;</span
></td><td id="665"><a href="#665">665</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_666"

 onmouseover="gutterOver(666)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',666);">&nbsp;</span
></td><td id="666"><a href="#666">666</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_667"

 onmouseover="gutterOver(667)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',667);">&nbsp;</span
></td><td id="667"><a href="#667">667</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_668"

 onmouseover="gutterOver(668)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',668);">&nbsp;</span
></td><td id="668"><a href="#668">668</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_669"

 onmouseover="gutterOver(669)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',669);">&nbsp;</span
></td><td id="669"><a href="#669">669</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_670"

 onmouseover="gutterOver(670)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',670);">&nbsp;</span
></td><td id="670"><a href="#670">670</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_671"

 onmouseover="gutterOver(671)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',671);">&nbsp;</span
></td><td id="671"><a href="#671">671</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_672"

 onmouseover="gutterOver(672)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',672);">&nbsp;</span
></td><td id="672"><a href="#672">672</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_673"

 onmouseover="gutterOver(673)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',673);">&nbsp;</span
></td><td id="673"><a href="#673">673</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_674"

 onmouseover="gutterOver(674)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',674);">&nbsp;</span
></td><td id="674"><a href="#674">674</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_675"

 onmouseover="gutterOver(675)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',675);">&nbsp;</span
></td><td id="675"><a href="#675">675</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_676"

 onmouseover="gutterOver(676)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',676);">&nbsp;</span
></td><td id="676"><a href="#676">676</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_677"

 onmouseover="gutterOver(677)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',677);">&nbsp;</span
></td><td id="677"><a href="#677">677</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_678"

 onmouseover="gutterOver(678)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',678);">&nbsp;</span
></td><td id="678"><a href="#678">678</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_679"

 onmouseover="gutterOver(679)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',679);">&nbsp;</span
></td><td id="679"><a href="#679">679</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_680"

 onmouseover="gutterOver(680)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',680);">&nbsp;</span
></td><td id="680"><a href="#680">680</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_681"

 onmouseover="gutterOver(681)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',681);">&nbsp;</span
></td><td id="681"><a href="#681">681</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_682"

 onmouseover="gutterOver(682)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',682);">&nbsp;</span
></td><td id="682"><a href="#682">682</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_683"

 onmouseover="gutterOver(683)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',683);">&nbsp;</span
></td><td id="683"><a href="#683">683</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_684"

 onmouseover="gutterOver(684)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',684);">&nbsp;</span
></td><td id="684"><a href="#684">684</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_685"

 onmouseover="gutterOver(685)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',685);">&nbsp;</span
></td><td id="685"><a href="#685">685</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_686"

 onmouseover="gutterOver(686)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',686);">&nbsp;</span
></td><td id="686"><a href="#686">686</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_687"

 onmouseover="gutterOver(687)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',687);">&nbsp;</span
></td><td id="687"><a href="#687">687</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_688"

 onmouseover="gutterOver(688)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',688);">&nbsp;</span
></td><td id="688"><a href="#688">688</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_689"

 onmouseover="gutterOver(689)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',689);">&nbsp;</span
></td><td id="689"><a href="#689">689</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_690"

 onmouseover="gutterOver(690)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',690);">&nbsp;</span
></td><td id="690"><a href="#690">690</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_691"

 onmouseover="gutterOver(691)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',691);">&nbsp;</span
></td><td id="691"><a href="#691">691</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_692"

 onmouseover="gutterOver(692)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',692);">&nbsp;</span
></td><td id="692"><a href="#692">692</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_693"

 onmouseover="gutterOver(693)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',693);">&nbsp;</span
></td><td id="693"><a href="#693">693</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_694"

 onmouseover="gutterOver(694)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',694);">&nbsp;</span
></td><td id="694"><a href="#694">694</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_695"

 onmouseover="gutterOver(695)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',695);">&nbsp;</span
></td><td id="695"><a href="#695">695</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_696"

 onmouseover="gutterOver(696)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',696);">&nbsp;</span
></td><td id="696"><a href="#696">696</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_697"

 onmouseover="gutterOver(697)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',697);">&nbsp;</span
></td><td id="697"><a href="#697">697</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_698"

 onmouseover="gutterOver(698)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',698);">&nbsp;</span
></td><td id="698"><a href="#698">698</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_699"

 onmouseover="gutterOver(699)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',699);">&nbsp;</span
></td><td id="699"><a href="#699">699</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_700"

 onmouseover="gutterOver(700)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',700);">&nbsp;</span
></td><td id="700"><a href="#700">700</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_701"

 onmouseover="gutterOver(701)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',701);">&nbsp;</span
></td><td id="701"><a href="#701">701</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_702"

 onmouseover="gutterOver(702)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',702);">&nbsp;</span
></td><td id="702"><a href="#702">702</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_703"

 onmouseover="gutterOver(703)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',703);">&nbsp;</span
></td><td id="703"><a href="#703">703</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_704"

 onmouseover="gutterOver(704)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',704);">&nbsp;</span
></td><td id="704"><a href="#704">704</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_705"

 onmouseover="gutterOver(705)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',705);">&nbsp;</span
></td><td id="705"><a href="#705">705</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_706"

 onmouseover="gutterOver(706)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',706);">&nbsp;</span
></td><td id="706"><a href="#706">706</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_707"

 onmouseover="gutterOver(707)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',707);">&nbsp;</span
></td><td id="707"><a href="#707">707</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_708"

 onmouseover="gutterOver(708)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',708);">&nbsp;</span
></td><td id="708"><a href="#708">708</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_709"

 onmouseover="gutterOver(709)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',709);">&nbsp;</span
></td><td id="709"><a href="#709">709</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_710"

 onmouseover="gutterOver(710)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',710);">&nbsp;</span
></td><td id="710"><a href="#710">710</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_711"

 onmouseover="gutterOver(711)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',711);">&nbsp;</span
></td><td id="711"><a href="#711">711</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_712"

 onmouseover="gutterOver(712)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',712);">&nbsp;</span
></td><td id="712"><a href="#712">712</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_713"

 onmouseover="gutterOver(713)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',713);">&nbsp;</span
></td><td id="713"><a href="#713">713</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_714"

 onmouseover="gutterOver(714)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',714);">&nbsp;</span
></td><td id="714"><a href="#714">714</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_715"

 onmouseover="gutterOver(715)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',715);">&nbsp;</span
></td><td id="715"><a href="#715">715</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_716"

 onmouseover="gutterOver(716)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',716);">&nbsp;</span
></td><td id="716"><a href="#716">716</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_717"

 onmouseover="gutterOver(717)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',717);">&nbsp;</span
></td><td id="717"><a href="#717">717</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_718"

 onmouseover="gutterOver(718)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',718);">&nbsp;</span
></td><td id="718"><a href="#718">718</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_719"

 onmouseover="gutterOver(719)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',719);">&nbsp;</span
></td><td id="719"><a href="#719">719</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_720"

 onmouseover="gutterOver(720)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',720);">&nbsp;</span
></td><td id="720"><a href="#720">720</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_721"

 onmouseover="gutterOver(721)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',721);">&nbsp;</span
></td><td id="721"><a href="#721">721</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_722"

 onmouseover="gutterOver(722)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',722);">&nbsp;</span
></td><td id="722"><a href="#722">722</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_723"

 onmouseover="gutterOver(723)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',723);">&nbsp;</span
></td><td id="723"><a href="#723">723</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_724"

 onmouseover="gutterOver(724)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',724);">&nbsp;</span
></td><td id="724"><a href="#724">724</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_725"

 onmouseover="gutterOver(725)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',725);">&nbsp;</span
></td><td id="725"><a href="#725">725</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_726"

 onmouseover="gutterOver(726)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',726);">&nbsp;</span
></td><td id="726"><a href="#726">726</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_727"

 onmouseover="gutterOver(727)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',727);">&nbsp;</span
></td><td id="727"><a href="#727">727</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_728"

 onmouseover="gutterOver(728)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',728);">&nbsp;</span
></td><td id="728"><a href="#728">728</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_729"

 onmouseover="gutterOver(729)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',729);">&nbsp;</span
></td><td id="729"><a href="#729">729</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_730"

 onmouseover="gutterOver(730)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',730);">&nbsp;</span
></td><td id="730"><a href="#730">730</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_731"

 onmouseover="gutterOver(731)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',731);">&nbsp;</span
></td><td id="731"><a href="#731">731</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_732"

 onmouseover="gutterOver(732)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',732);">&nbsp;</span
></td><td id="732"><a href="#732">732</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_733"

 onmouseover="gutterOver(733)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',733);">&nbsp;</span
></td><td id="733"><a href="#733">733</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_734"

 onmouseover="gutterOver(734)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',734);">&nbsp;</span
></td><td id="734"><a href="#734">734</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_735"

 onmouseover="gutterOver(735)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',735);">&nbsp;</span
></td><td id="735"><a href="#735">735</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_736"

 onmouseover="gutterOver(736)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',736);">&nbsp;</span
></td><td id="736"><a href="#736">736</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_737"

 onmouseover="gutterOver(737)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',737);">&nbsp;</span
></td><td id="737"><a href="#737">737</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_738"

 onmouseover="gutterOver(738)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',738);">&nbsp;</span
></td><td id="738"><a href="#738">738</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_739"

 onmouseover="gutterOver(739)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',739);">&nbsp;</span
></td><td id="739"><a href="#739">739</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_740"

 onmouseover="gutterOver(740)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',740);">&nbsp;</span
></td><td id="740"><a href="#740">740</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_741"

 onmouseover="gutterOver(741)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',741);">&nbsp;</span
></td><td id="741"><a href="#741">741</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_742"

 onmouseover="gutterOver(742)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',742);">&nbsp;</span
></td><td id="742"><a href="#742">742</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_743"

 onmouseover="gutterOver(743)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',743);">&nbsp;</span
></td><td id="743"><a href="#743">743</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_744"

 onmouseover="gutterOver(744)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',744);">&nbsp;</span
></td><td id="744"><a href="#744">744</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_745"

 onmouseover="gutterOver(745)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',745);">&nbsp;</span
></td><td id="745"><a href="#745">745</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_746"

 onmouseover="gutterOver(746)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',746);">&nbsp;</span
></td><td id="746"><a href="#746">746</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_747"

 onmouseover="gutterOver(747)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',747);">&nbsp;</span
></td><td id="747"><a href="#747">747</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_748"

 onmouseover="gutterOver(748)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',748);">&nbsp;</span
></td><td id="748"><a href="#748">748</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_749"

 onmouseover="gutterOver(749)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',749);">&nbsp;</span
></td><td id="749"><a href="#749">749</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_750"

 onmouseover="gutterOver(750)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',750);">&nbsp;</span
></td><td id="750"><a href="#750">750</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_751"

 onmouseover="gutterOver(751)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',751);">&nbsp;</span
></td><td id="751"><a href="#751">751</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_752"

 onmouseover="gutterOver(752)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',752);">&nbsp;</span
></td><td id="752"><a href="#752">752</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_753"

 onmouseover="gutterOver(753)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',753);">&nbsp;</span
></td><td id="753"><a href="#753">753</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_754"

 onmouseover="gutterOver(754)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',754);">&nbsp;</span
></td><td id="754"><a href="#754">754</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_755"

 onmouseover="gutterOver(755)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',755);">&nbsp;</span
></td><td id="755"><a href="#755">755</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_756"

 onmouseover="gutterOver(756)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',756);">&nbsp;</span
></td><td id="756"><a href="#756">756</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_757"

 onmouseover="gutterOver(757)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',757);">&nbsp;</span
></td><td id="757"><a href="#757">757</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_758"

 onmouseover="gutterOver(758)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',758);">&nbsp;</span
></td><td id="758"><a href="#758">758</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_759"

 onmouseover="gutterOver(759)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',759);">&nbsp;</span
></td><td id="759"><a href="#759">759</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_760"

 onmouseover="gutterOver(760)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',760);">&nbsp;</span
></td><td id="760"><a href="#760">760</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_761"

 onmouseover="gutterOver(761)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',761);">&nbsp;</span
></td><td id="761"><a href="#761">761</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_762"

 onmouseover="gutterOver(762)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',762);">&nbsp;</span
></td><td id="762"><a href="#762">762</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_763"

 onmouseover="gutterOver(763)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',763);">&nbsp;</span
></td><td id="763"><a href="#763">763</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_764"

 onmouseover="gutterOver(764)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',764);">&nbsp;</span
></td><td id="764"><a href="#764">764</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_765"

 onmouseover="gutterOver(765)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',765);">&nbsp;</span
></td><td id="765"><a href="#765">765</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_766"

 onmouseover="gutterOver(766)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',766);">&nbsp;</span
></td><td id="766"><a href="#766">766</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_767"

 onmouseover="gutterOver(767)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',767);">&nbsp;</span
></td><td id="767"><a href="#767">767</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_768"

 onmouseover="gutterOver(768)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',768);">&nbsp;</span
></td><td id="768"><a href="#768">768</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_769"

 onmouseover="gutterOver(769)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',769);">&nbsp;</span
></td><td id="769"><a href="#769">769</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_770"

 onmouseover="gutterOver(770)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',770);">&nbsp;</span
></td><td id="770"><a href="#770">770</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_771"

 onmouseover="gutterOver(771)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',771);">&nbsp;</span
></td><td id="771"><a href="#771">771</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_772"

 onmouseover="gutterOver(772)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',772);">&nbsp;</span
></td><td id="772"><a href="#772">772</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_773"

 onmouseover="gutterOver(773)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',773);">&nbsp;</span
></td><td id="773"><a href="#773">773</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_774"

 onmouseover="gutterOver(774)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',774);">&nbsp;</span
></td><td id="774"><a href="#774">774</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_775"

 onmouseover="gutterOver(775)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',775);">&nbsp;</span
></td><td id="775"><a href="#775">775</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_776"

 onmouseover="gutterOver(776)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',776);">&nbsp;</span
></td><td id="776"><a href="#776">776</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_777"

 onmouseover="gutterOver(777)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',777);">&nbsp;</span
></td><td id="777"><a href="#777">777</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_778"

 onmouseover="gutterOver(778)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',778);">&nbsp;</span
></td><td id="778"><a href="#778">778</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_779"

 onmouseover="gutterOver(779)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',779);">&nbsp;</span
></td><td id="779"><a href="#779">779</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_780"

 onmouseover="gutterOver(780)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',780);">&nbsp;</span
></td><td id="780"><a href="#780">780</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_781"

 onmouseover="gutterOver(781)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',781);">&nbsp;</span
></td><td id="781"><a href="#781">781</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_782"

 onmouseover="gutterOver(782)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',782);">&nbsp;</span
></td><td id="782"><a href="#782">782</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_783"

 onmouseover="gutterOver(783)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',783);">&nbsp;</span
></td><td id="783"><a href="#783">783</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_784"

 onmouseover="gutterOver(784)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',784);">&nbsp;</span
></td><td id="784"><a href="#784">784</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_785"

 onmouseover="gutterOver(785)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',785);">&nbsp;</span
></td><td id="785"><a href="#785">785</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_786"

 onmouseover="gutterOver(786)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',786);">&nbsp;</span
></td><td id="786"><a href="#786">786</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_787"

 onmouseover="gutterOver(787)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',787);">&nbsp;</span
></td><td id="787"><a href="#787">787</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_788"

 onmouseover="gutterOver(788)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',788);">&nbsp;</span
></td><td id="788"><a href="#788">788</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_789"

 onmouseover="gutterOver(789)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',789);">&nbsp;</span
></td><td id="789"><a href="#789">789</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_790"

 onmouseover="gutterOver(790)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',790);">&nbsp;</span
></td><td id="790"><a href="#790">790</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_791"

 onmouseover="gutterOver(791)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',791);">&nbsp;</span
></td><td id="791"><a href="#791">791</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_792"

 onmouseover="gutterOver(792)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',792);">&nbsp;</span
></td><td id="792"><a href="#792">792</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_793"

 onmouseover="gutterOver(793)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',793);">&nbsp;</span
></td><td id="793"><a href="#793">793</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_794"

 onmouseover="gutterOver(794)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',794);">&nbsp;</span
></td><td id="794"><a href="#794">794</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_795"

 onmouseover="gutterOver(795)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',795);">&nbsp;</span
></td><td id="795"><a href="#795">795</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_796"

 onmouseover="gutterOver(796)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',796);">&nbsp;</span
></td><td id="796"><a href="#796">796</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_797"

 onmouseover="gutterOver(797)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',797);">&nbsp;</span
></td><td id="797"><a href="#797">797</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_798"

 onmouseover="gutterOver(798)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',798);">&nbsp;</span
></td><td id="798"><a href="#798">798</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_799"

 onmouseover="gutterOver(799)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',799);">&nbsp;</span
></td><td id="799"><a href="#799">799</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_800"

 onmouseover="gutterOver(800)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',800);">&nbsp;</span
></td><td id="800"><a href="#800">800</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_801"

 onmouseover="gutterOver(801)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',801);">&nbsp;</span
></td><td id="801"><a href="#801">801</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_802"

 onmouseover="gutterOver(802)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',802);">&nbsp;</span
></td><td id="802"><a href="#802">802</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_803"

 onmouseover="gutterOver(803)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',803);">&nbsp;</span
></td><td id="803"><a href="#803">803</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_804"

 onmouseover="gutterOver(804)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',804);">&nbsp;</span
></td><td id="804"><a href="#804">804</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_805"

 onmouseover="gutterOver(805)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',805);">&nbsp;</span
></td><td id="805"><a href="#805">805</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_806"

 onmouseover="gutterOver(806)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',806);">&nbsp;</span
></td><td id="806"><a href="#806">806</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_807"

 onmouseover="gutterOver(807)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',807);">&nbsp;</span
></td><td id="807"><a href="#807">807</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_808"

 onmouseover="gutterOver(808)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',808);">&nbsp;</span
></td><td id="808"><a href="#808">808</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_809"

 onmouseover="gutterOver(809)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',809);">&nbsp;</span
></td><td id="809"><a href="#809">809</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_810"

 onmouseover="gutterOver(810)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',810);">&nbsp;</span
></td><td id="810"><a href="#810">810</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_811"

 onmouseover="gutterOver(811)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',811);">&nbsp;</span
></td><td id="811"><a href="#811">811</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_812"

 onmouseover="gutterOver(812)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',812);">&nbsp;</span
></td><td id="812"><a href="#812">812</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_813"

 onmouseover="gutterOver(813)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',813);">&nbsp;</span
></td><td id="813"><a href="#813">813</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_814"

 onmouseover="gutterOver(814)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',814);">&nbsp;</span
></td><td id="814"><a href="#814">814</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_815"

 onmouseover="gutterOver(815)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',815);">&nbsp;</span
></td><td id="815"><a href="#815">815</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_816"

 onmouseover="gutterOver(816)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',816);">&nbsp;</span
></td><td id="816"><a href="#816">816</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_817"

 onmouseover="gutterOver(817)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',817);">&nbsp;</span
></td><td id="817"><a href="#817">817</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_818"

 onmouseover="gutterOver(818)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',818);">&nbsp;</span
></td><td id="818"><a href="#818">818</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_819"

 onmouseover="gutterOver(819)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',819);">&nbsp;</span
></td><td id="819"><a href="#819">819</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_820"

 onmouseover="gutterOver(820)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',820);">&nbsp;</span
></td><td id="820"><a href="#820">820</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_821"

 onmouseover="gutterOver(821)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',821);">&nbsp;</span
></td><td id="821"><a href="#821">821</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_822"

 onmouseover="gutterOver(822)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',822);">&nbsp;</span
></td><td id="822"><a href="#822">822</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_823"

 onmouseover="gutterOver(823)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',823);">&nbsp;</span
></td><td id="823"><a href="#823">823</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_824"

 onmouseover="gutterOver(824)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',824);">&nbsp;</span
></td><td id="824"><a href="#824">824</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_825"

 onmouseover="gutterOver(825)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',825);">&nbsp;</span
></td><td id="825"><a href="#825">825</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_826"

 onmouseover="gutterOver(826)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',826);">&nbsp;</span
></td><td id="826"><a href="#826">826</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_827"

 onmouseover="gutterOver(827)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',827);">&nbsp;</span
></td><td id="827"><a href="#827">827</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_828"

 onmouseover="gutterOver(828)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',828);">&nbsp;</span
></td><td id="828"><a href="#828">828</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_829"

 onmouseover="gutterOver(829)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',829);">&nbsp;</span
></td><td id="829"><a href="#829">829</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_830"

 onmouseover="gutterOver(830)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',830);">&nbsp;</span
></td><td id="830"><a href="#830">830</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_831"

 onmouseover="gutterOver(831)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',831);">&nbsp;</span
></td><td id="831"><a href="#831">831</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_832"

 onmouseover="gutterOver(832)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',832);">&nbsp;</span
></td><td id="832"><a href="#832">832</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_833"

 onmouseover="gutterOver(833)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',833);">&nbsp;</span
></td><td id="833"><a href="#833">833</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_834"

 onmouseover="gutterOver(834)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',834);">&nbsp;</span
></td><td id="834"><a href="#834">834</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_835"

 onmouseover="gutterOver(835)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',835);">&nbsp;</span
></td><td id="835"><a href="#835">835</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_836"

 onmouseover="gutterOver(836)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',836);">&nbsp;</span
></td><td id="836"><a href="#836">836</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_837"

 onmouseover="gutterOver(837)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',837);">&nbsp;</span
></td><td id="837"><a href="#837">837</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_838"

 onmouseover="gutterOver(838)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',838);">&nbsp;</span
></td><td id="838"><a href="#838">838</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_839"

 onmouseover="gutterOver(839)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',839);">&nbsp;</span
></td><td id="839"><a href="#839">839</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_840"

 onmouseover="gutterOver(840)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',840);">&nbsp;</span
></td><td id="840"><a href="#840">840</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_841"

 onmouseover="gutterOver(841)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',841);">&nbsp;</span
></td><td id="841"><a href="#841">841</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_842"

 onmouseover="gutterOver(842)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',842);">&nbsp;</span
></td><td id="842"><a href="#842">842</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_843"

 onmouseover="gutterOver(843)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',843);">&nbsp;</span
></td><td id="843"><a href="#843">843</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_844"

 onmouseover="gutterOver(844)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',844);">&nbsp;</span
></td><td id="844"><a href="#844">844</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_845"

 onmouseover="gutterOver(845)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',845);">&nbsp;</span
></td><td id="845"><a href="#845">845</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_846"

 onmouseover="gutterOver(846)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',846);">&nbsp;</span
></td><td id="846"><a href="#846">846</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_847"

 onmouseover="gutterOver(847)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',847);">&nbsp;</span
></td><td id="847"><a href="#847">847</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_848"

 onmouseover="gutterOver(848)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',848);">&nbsp;</span
></td><td id="848"><a href="#848">848</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_849"

 onmouseover="gutterOver(849)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',849);">&nbsp;</span
></td><td id="849"><a href="#849">849</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_850"

 onmouseover="gutterOver(850)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',850);">&nbsp;</span
></td><td id="850"><a href="#850">850</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_851"

 onmouseover="gutterOver(851)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',851);">&nbsp;</span
></td><td id="851"><a href="#851">851</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_852"

 onmouseover="gutterOver(852)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',852);">&nbsp;</span
></td><td id="852"><a href="#852">852</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_853"

 onmouseover="gutterOver(853)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',853);">&nbsp;</span
></td><td id="853"><a href="#853">853</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_854"

 onmouseover="gutterOver(854)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',854);">&nbsp;</span
></td><td id="854"><a href="#854">854</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_855"

 onmouseover="gutterOver(855)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',855);">&nbsp;</span
></td><td id="855"><a href="#855">855</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_856"

 onmouseover="gutterOver(856)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',856);">&nbsp;</span
></td><td id="856"><a href="#856">856</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_857"

 onmouseover="gutterOver(857)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',857);">&nbsp;</span
></td><td id="857"><a href="#857">857</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_858"

 onmouseover="gutterOver(858)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',858);">&nbsp;</span
></td><td id="858"><a href="#858">858</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_859"

 onmouseover="gutterOver(859)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',859);">&nbsp;</span
></td><td id="859"><a href="#859">859</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_860"

 onmouseover="gutterOver(860)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',860);">&nbsp;</span
></td><td id="860"><a href="#860">860</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_861"

 onmouseover="gutterOver(861)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',861);">&nbsp;</span
></td><td id="861"><a href="#861">861</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_862"

 onmouseover="gutterOver(862)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',862);">&nbsp;</span
></td><td id="862"><a href="#862">862</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_863"

 onmouseover="gutterOver(863)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',863);">&nbsp;</span
></td><td id="863"><a href="#863">863</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_864"

 onmouseover="gutterOver(864)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',864);">&nbsp;</span
></td><td id="864"><a href="#864">864</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_865"

 onmouseover="gutterOver(865)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',865);">&nbsp;</span
></td><td id="865"><a href="#865">865</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_866"

 onmouseover="gutterOver(866)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',866);">&nbsp;</span
></td><td id="866"><a href="#866">866</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_867"

 onmouseover="gutterOver(867)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',867);">&nbsp;</span
></td><td id="867"><a href="#867">867</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_868"

 onmouseover="gutterOver(868)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',868);">&nbsp;</span
></td><td id="868"><a href="#868">868</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_869"

 onmouseover="gutterOver(869)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',869);">&nbsp;</span
></td><td id="869"><a href="#869">869</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_870"

 onmouseover="gutterOver(870)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',870);">&nbsp;</span
></td><td id="870"><a href="#870">870</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_871"

 onmouseover="gutterOver(871)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',871);">&nbsp;</span
></td><td id="871"><a href="#871">871</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_872"

 onmouseover="gutterOver(872)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',872);">&nbsp;</span
></td><td id="872"><a href="#872">872</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_873"

 onmouseover="gutterOver(873)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',873);">&nbsp;</span
></td><td id="873"><a href="#873">873</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_874"

 onmouseover="gutterOver(874)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',874);">&nbsp;</span
></td><td id="874"><a href="#874">874</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_875"

 onmouseover="gutterOver(875)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',875);">&nbsp;</span
></td><td id="875"><a href="#875">875</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_876"

 onmouseover="gutterOver(876)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',876);">&nbsp;</span
></td><td id="876"><a href="#876">876</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_877"

 onmouseover="gutterOver(877)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',877);">&nbsp;</span
></td><td id="877"><a href="#877">877</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_878"

 onmouseover="gutterOver(878)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',878);">&nbsp;</span
></td><td id="878"><a href="#878">878</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_879"

 onmouseover="gutterOver(879)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',879);">&nbsp;</span
></td><td id="879"><a href="#879">879</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_880"

 onmouseover="gutterOver(880)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',880);">&nbsp;</span
></td><td id="880"><a href="#880">880</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_881"

 onmouseover="gutterOver(881)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',881);">&nbsp;</span
></td><td id="881"><a href="#881">881</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_882"

 onmouseover="gutterOver(882)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',882);">&nbsp;</span
></td><td id="882"><a href="#882">882</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_883"

 onmouseover="gutterOver(883)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',883);">&nbsp;</span
></td><td id="883"><a href="#883">883</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_884"

 onmouseover="gutterOver(884)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',884);">&nbsp;</span
></td><td id="884"><a href="#884">884</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_885"

 onmouseover="gutterOver(885)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',885);">&nbsp;</span
></td><td id="885"><a href="#885">885</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_886"

 onmouseover="gutterOver(886)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',886);">&nbsp;</span
></td><td id="886"><a href="#886">886</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_887"

 onmouseover="gutterOver(887)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',887);">&nbsp;</span
></td><td id="887"><a href="#887">887</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_888"

 onmouseover="gutterOver(888)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',888);">&nbsp;</span
></td><td id="888"><a href="#888">888</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_889"

 onmouseover="gutterOver(889)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',889);">&nbsp;</span
></td><td id="889"><a href="#889">889</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_890"

 onmouseover="gutterOver(890)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',890);">&nbsp;</span
></td><td id="890"><a href="#890">890</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_891"

 onmouseover="gutterOver(891)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',891);">&nbsp;</span
></td><td id="891"><a href="#891">891</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_892"

 onmouseover="gutterOver(892)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',892);">&nbsp;</span
></td><td id="892"><a href="#892">892</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_893"

 onmouseover="gutterOver(893)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',893);">&nbsp;</span
></td><td id="893"><a href="#893">893</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_894"

 onmouseover="gutterOver(894)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',894);">&nbsp;</span
></td><td id="894"><a href="#894">894</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_895"

 onmouseover="gutterOver(895)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',895);">&nbsp;</span
></td><td id="895"><a href="#895">895</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_896"

 onmouseover="gutterOver(896)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',896);">&nbsp;</span
></td><td id="896"><a href="#896">896</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_897"

 onmouseover="gutterOver(897)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',897);">&nbsp;</span
></td><td id="897"><a href="#897">897</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_898"

 onmouseover="gutterOver(898)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',898);">&nbsp;</span
></td><td id="898"><a href="#898">898</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_899"

 onmouseover="gutterOver(899)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',899);">&nbsp;</span
></td><td id="899"><a href="#899">899</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_900"

 onmouseover="gutterOver(900)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',900);">&nbsp;</span
></td><td id="900"><a href="#900">900</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_901"

 onmouseover="gutterOver(901)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',901);">&nbsp;</span
></td><td id="901"><a href="#901">901</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_902"

 onmouseover="gutterOver(902)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',902);">&nbsp;</span
></td><td id="902"><a href="#902">902</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_903"

 onmouseover="gutterOver(903)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',903);">&nbsp;</span
></td><td id="903"><a href="#903">903</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_904"

 onmouseover="gutterOver(904)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',904);">&nbsp;</span
></td><td id="904"><a href="#904">904</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_905"

 onmouseover="gutterOver(905)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',905);">&nbsp;</span
></td><td id="905"><a href="#905">905</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_906"

 onmouseover="gutterOver(906)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',906);">&nbsp;</span
></td><td id="906"><a href="#906">906</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_907"

 onmouseover="gutterOver(907)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',907);">&nbsp;</span
></td><td id="907"><a href="#907">907</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_908"

 onmouseover="gutterOver(908)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',908);">&nbsp;</span
></td><td id="908"><a href="#908">908</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_909"

 onmouseover="gutterOver(909)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',909);">&nbsp;</span
></td><td id="909"><a href="#909">909</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_910"

 onmouseover="gutterOver(910)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',910);">&nbsp;</span
></td><td id="910"><a href="#910">910</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_911"

 onmouseover="gutterOver(911)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',911);">&nbsp;</span
></td><td id="911"><a href="#911">911</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_912"

 onmouseover="gutterOver(912)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',912);">&nbsp;</span
></td><td id="912"><a href="#912">912</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_913"

 onmouseover="gutterOver(913)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',913);">&nbsp;</span
></td><td id="913"><a href="#913">913</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_914"

 onmouseover="gutterOver(914)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',914);">&nbsp;</span
></td><td id="914"><a href="#914">914</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_915"

 onmouseover="gutterOver(915)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',915);">&nbsp;</span
></td><td id="915"><a href="#915">915</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_916"

 onmouseover="gutterOver(916)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',916);">&nbsp;</span
></td><td id="916"><a href="#916">916</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_917"

 onmouseover="gutterOver(917)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',917);">&nbsp;</span
></td><td id="917"><a href="#917">917</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_918"

 onmouseover="gutterOver(918)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',918);">&nbsp;</span
></td><td id="918"><a href="#918">918</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_919"

 onmouseover="gutterOver(919)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',919);">&nbsp;</span
></td><td id="919"><a href="#919">919</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_920"

 onmouseover="gutterOver(920)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',920);">&nbsp;</span
></td><td id="920"><a href="#920">920</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_921"

 onmouseover="gutterOver(921)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',921);">&nbsp;</span
></td><td id="921"><a href="#921">921</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_922"

 onmouseover="gutterOver(922)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',922);">&nbsp;</span
></td><td id="922"><a href="#922">922</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_923"

 onmouseover="gutterOver(923)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',923);">&nbsp;</span
></td><td id="923"><a href="#923">923</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_924"

 onmouseover="gutterOver(924)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',924);">&nbsp;</span
></td><td id="924"><a href="#924">924</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_925"

 onmouseover="gutterOver(925)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',925);">&nbsp;</span
></td><td id="925"><a href="#925">925</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_926"

 onmouseover="gutterOver(926)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',926);">&nbsp;</span
></td><td id="926"><a href="#926">926</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_927"

 onmouseover="gutterOver(927)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',927);">&nbsp;</span
></td><td id="927"><a href="#927">927</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_928"

 onmouseover="gutterOver(928)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',928);">&nbsp;</span
></td><td id="928"><a href="#928">928</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_929"

 onmouseover="gutterOver(929)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',929);">&nbsp;</span
></td><td id="929"><a href="#929">929</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_930"

 onmouseover="gutterOver(930)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',930);">&nbsp;</span
></td><td id="930"><a href="#930">930</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_931"

 onmouseover="gutterOver(931)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',931);">&nbsp;</span
></td><td id="931"><a href="#931">931</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_932"

 onmouseover="gutterOver(932)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',932);">&nbsp;</span
></td><td id="932"><a href="#932">932</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_933"

 onmouseover="gutterOver(933)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',933);">&nbsp;</span
></td><td id="933"><a href="#933">933</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_934"

 onmouseover="gutterOver(934)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',934);">&nbsp;</span
></td><td id="934"><a href="#934">934</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_935"

 onmouseover="gutterOver(935)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',935);">&nbsp;</span
></td><td id="935"><a href="#935">935</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_936"

 onmouseover="gutterOver(936)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',936);">&nbsp;</span
></td><td id="936"><a href="#936">936</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_937"

 onmouseover="gutterOver(937)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',937);">&nbsp;</span
></td><td id="937"><a href="#937">937</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_938"

 onmouseover="gutterOver(938)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',938);">&nbsp;</span
></td><td id="938"><a href="#938">938</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_939"

 onmouseover="gutterOver(939)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',939);">&nbsp;</span
></td><td id="939"><a href="#939">939</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_940"

 onmouseover="gutterOver(940)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',940);">&nbsp;</span
></td><td id="940"><a href="#940">940</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_941"

 onmouseover="gutterOver(941)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',941);">&nbsp;</span
></td><td id="941"><a href="#941">941</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_942"

 onmouseover="gutterOver(942)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',942);">&nbsp;</span
></td><td id="942"><a href="#942">942</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_943"

 onmouseover="gutterOver(943)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',943);">&nbsp;</span
></td><td id="943"><a href="#943">943</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_944"

 onmouseover="gutterOver(944)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',944);">&nbsp;</span
></td><td id="944"><a href="#944">944</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_945"

 onmouseover="gutterOver(945)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',945);">&nbsp;</span
></td><td id="945"><a href="#945">945</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_946"

 onmouseover="gutterOver(946)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',946);">&nbsp;</span
></td><td id="946"><a href="#946">946</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_947"

 onmouseover="gutterOver(947)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',947);">&nbsp;</span
></td><td id="947"><a href="#947">947</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_948"

 onmouseover="gutterOver(948)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',948);">&nbsp;</span
></td><td id="948"><a href="#948">948</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_949"

 onmouseover="gutterOver(949)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',949);">&nbsp;</span
></td><td id="949"><a href="#949">949</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_950"

 onmouseover="gutterOver(950)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',950);">&nbsp;</span
></td><td id="950"><a href="#950">950</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_951"

 onmouseover="gutterOver(951)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',951);">&nbsp;</span
></td><td id="951"><a href="#951">951</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_952"

 onmouseover="gutterOver(952)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',952);">&nbsp;</span
></td><td id="952"><a href="#952">952</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_953"

 onmouseover="gutterOver(953)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',953);">&nbsp;</span
></td><td id="953"><a href="#953">953</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_954"

 onmouseover="gutterOver(954)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',954);">&nbsp;</span
></td><td id="954"><a href="#954">954</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_955"

 onmouseover="gutterOver(955)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',955);">&nbsp;</span
></td><td id="955"><a href="#955">955</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_956"

 onmouseover="gutterOver(956)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',956);">&nbsp;</span
></td><td id="956"><a href="#956">956</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_957"

 onmouseover="gutterOver(957)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',957);">&nbsp;</span
></td><td id="957"><a href="#957">957</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_958"

 onmouseover="gutterOver(958)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',958);">&nbsp;</span
></td><td id="958"><a href="#958">958</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_959"

 onmouseover="gutterOver(959)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',959);">&nbsp;</span
></td><td id="959"><a href="#959">959</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_960"

 onmouseover="gutterOver(960)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',960);">&nbsp;</span
></td><td id="960"><a href="#960">960</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_961"

 onmouseover="gutterOver(961)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',961);">&nbsp;</span
></td><td id="961"><a href="#961">961</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_962"

 onmouseover="gutterOver(962)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',962);">&nbsp;</span
></td><td id="962"><a href="#962">962</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_963"

 onmouseover="gutterOver(963)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',963);">&nbsp;</span
></td><td id="963"><a href="#963">963</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_964"

 onmouseover="gutterOver(964)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',964);">&nbsp;</span
></td><td id="964"><a href="#964">964</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_965"

 onmouseover="gutterOver(965)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',965);">&nbsp;</span
></td><td id="965"><a href="#965">965</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_966"

 onmouseover="gutterOver(966)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',966);">&nbsp;</span
></td><td id="966"><a href="#966">966</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_967"

 onmouseover="gutterOver(967)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',967);">&nbsp;</span
></td><td id="967"><a href="#967">967</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_968"

 onmouseover="gutterOver(968)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',968);">&nbsp;</span
></td><td id="968"><a href="#968">968</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_969"

 onmouseover="gutterOver(969)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',969);">&nbsp;</span
></td><td id="969"><a href="#969">969</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_970"

 onmouseover="gutterOver(970)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',970);">&nbsp;</span
></td><td id="970"><a href="#970">970</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_971"

 onmouseover="gutterOver(971)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',971);">&nbsp;</span
></td><td id="971"><a href="#971">971</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_972"

 onmouseover="gutterOver(972)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',972);">&nbsp;</span
></td><td id="972"><a href="#972">972</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_973"

 onmouseover="gutterOver(973)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',973);">&nbsp;</span
></td><td id="973"><a href="#973">973</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_974"

 onmouseover="gutterOver(974)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',974);">&nbsp;</span
></td><td id="974"><a href="#974">974</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_975"

 onmouseover="gutterOver(975)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',975);">&nbsp;</span
></td><td id="975"><a href="#975">975</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_976"

 onmouseover="gutterOver(976)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',976);">&nbsp;</span
></td><td id="976"><a href="#976">976</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_977"

 onmouseover="gutterOver(977)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',977);">&nbsp;</span
></td><td id="977"><a href="#977">977</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_978"

 onmouseover="gutterOver(978)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',978);">&nbsp;</span
></td><td id="978"><a href="#978">978</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_979"

 onmouseover="gutterOver(979)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',979);">&nbsp;</span
></td><td id="979"><a href="#979">979</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_980"

 onmouseover="gutterOver(980)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',980);">&nbsp;</span
></td><td id="980"><a href="#980">980</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_981"

 onmouseover="gutterOver(981)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',981);">&nbsp;</span
></td><td id="981"><a href="#981">981</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_982"

 onmouseover="gutterOver(982)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',982);">&nbsp;</span
></td><td id="982"><a href="#982">982</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_983"

 onmouseover="gutterOver(983)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',983);">&nbsp;</span
></td><td id="983"><a href="#983">983</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_984"

 onmouseover="gutterOver(984)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',984);">&nbsp;</span
></td><td id="984"><a href="#984">984</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_985"

 onmouseover="gutterOver(985)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',985);">&nbsp;</span
></td><td id="985"><a href="#985">985</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_986"

 onmouseover="gutterOver(986)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',986);">&nbsp;</span
></td><td id="986"><a href="#986">986</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_987"

 onmouseover="gutterOver(987)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',987);">&nbsp;</span
></td><td id="987"><a href="#987">987</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_988"

 onmouseover="gutterOver(988)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',988);">&nbsp;</span
></td><td id="988"><a href="#988">988</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_989"

 onmouseover="gutterOver(989)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',989);">&nbsp;</span
></td><td id="989"><a href="#989">989</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_990"

 onmouseover="gutterOver(990)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',990);">&nbsp;</span
></td><td id="990"><a href="#990">990</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_991"

 onmouseover="gutterOver(991)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',991);">&nbsp;</span
></td><td id="991"><a href="#991">991</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_992"

 onmouseover="gutterOver(992)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',992);">&nbsp;</span
></td><td id="992"><a href="#992">992</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_993"

 onmouseover="gutterOver(993)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',993);">&nbsp;</span
></td><td id="993"><a href="#993">993</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_994"

 onmouseover="gutterOver(994)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',994);">&nbsp;</span
></td><td id="994"><a href="#994">994</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_995"

 onmouseover="gutterOver(995)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',995);">&nbsp;</span
></td><td id="995"><a href="#995">995</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_996"

 onmouseover="gutterOver(996)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',996);">&nbsp;</span
></td><td id="996"><a href="#996">996</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_997"

 onmouseover="gutterOver(997)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',997);">&nbsp;</span
></td><td id="997"><a href="#997">997</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_998"

 onmouseover="gutterOver(998)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',998);">&nbsp;</span
></td><td id="998"><a href="#998">998</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_999"

 onmouseover="gutterOver(999)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',999);">&nbsp;</span
></td><td id="999"><a href="#999">999</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1000"

 onmouseover="gutterOver(1000)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1000);">&nbsp;</span
></td><td id="1000"><a href="#1000">1000</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1001"

 onmouseover="gutterOver(1001)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1001);">&nbsp;</span
></td><td id="1001"><a href="#1001">1001</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1002"

 onmouseover="gutterOver(1002)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1002);">&nbsp;</span
></td><td id="1002"><a href="#1002">1002</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1003"

 onmouseover="gutterOver(1003)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1003);">&nbsp;</span
></td><td id="1003"><a href="#1003">1003</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1004"

 onmouseover="gutterOver(1004)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1004);">&nbsp;</span
></td><td id="1004"><a href="#1004">1004</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1005"

 onmouseover="gutterOver(1005)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1005);">&nbsp;</span
></td><td id="1005"><a href="#1005">1005</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1006"

 onmouseover="gutterOver(1006)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1006);">&nbsp;</span
></td><td id="1006"><a href="#1006">1006</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1007"

 onmouseover="gutterOver(1007)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1007);">&nbsp;</span
></td><td id="1007"><a href="#1007">1007</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1008"

 onmouseover="gutterOver(1008)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1008);">&nbsp;</span
></td><td id="1008"><a href="#1008">1008</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1009"

 onmouseover="gutterOver(1009)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1009);">&nbsp;</span
></td><td id="1009"><a href="#1009">1009</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1010"

 onmouseover="gutterOver(1010)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1010);">&nbsp;</span
></td><td id="1010"><a href="#1010">1010</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1011"

 onmouseover="gutterOver(1011)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1011);">&nbsp;</span
></td><td id="1011"><a href="#1011">1011</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1012"

 onmouseover="gutterOver(1012)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1012);">&nbsp;</span
></td><td id="1012"><a href="#1012">1012</a></td></tr
><tr id="gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1013"

 onmouseover="gutterOver(1013)"

><td><span title="Add comment" onclick="codereviews.startEdit('svn06de7ef9f7907b57123b0e03476e547a1db6ae08',1013);">&nbsp;</span
></td><td id="1013"><a href="#1013">1013</a></td></tr
></table></pre>
<pre><table width="100%"><tr class="nocursor"><td></td></tr></table></pre>
</td>
<td id="lines">
<pre><table width="100%"><tr class="cursor_stop cursor_hidden"><td></td></tr></table></pre>
<pre class="prettyprint lang-cpp"><table id="src_table_0"><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1

 onmouseover="gutterOver(1)"

><td class="source">/*-------------------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_2

 onmouseover="gutterOver(2)"

><td class="source">Copyright (c) 2006 John Judnich<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_3

 onmouseover="gutterOver(3)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_4

 onmouseover="gutterOver(4)"

><td class="source">This software is provided &#39;as-is&#39;, without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_5

 onmouseover="gutterOver(5)"

><td class="source">Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_6

 onmouseover="gutterOver(6)"

><td class="source">1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_7

 onmouseover="gutterOver(7)"

><td class="source">2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_8

 onmouseover="gutterOver(8)"

><td class="source">3. This notice may not be removed or altered from any source distribution.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_9

 onmouseover="gutterOver(9)"

><td class="source">-------------------------------------------------------------------------------------*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_10

 onmouseover="gutterOver(10)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_11

 onmouseover="gutterOver(11)"

><td class="source">//BatchedGeometry.h<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_12

 onmouseover="gutterOver(12)"

><td class="source">//A &quot;lightweight&quot; version of Ogre::StaticGeometry, which gives you a little more control<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_13

 onmouseover="gutterOver(13)"

><td class="source">//over the batch materials, etc.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_14

 onmouseover="gutterOver(14)"

><td class="source">//-------------------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_15

 onmouseover="gutterOver(15)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_16

 onmouseover="gutterOver(16)"

><td class="source">#include &lt;OgreRoot.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_17

 onmouseover="gutterOver(17)"

><td class="source">#include &lt;OgreRenderSystem.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_18

 onmouseover="gutterOver(18)"

><td class="source">#include &lt;OgreCamera.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_19

 onmouseover="gutterOver(19)"

><td class="source">#include &lt;OgreVector3.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_20

 onmouseover="gutterOver(20)"

><td class="source">#include &lt;OgreQuaternion.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_21

 onmouseover="gutterOver(21)"

><td class="source">#include &lt;OgreSceneNode.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_22

 onmouseover="gutterOver(22)"

><td class="source">#include &lt;OgreString.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_23

 onmouseover="gutterOver(23)"

><td class="source">#include &lt;OgreStringConverter.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_24

 onmouseover="gutterOver(24)"

><td class="source">#include &lt;OgreEntity.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_25

 onmouseover="gutterOver(25)"

><td class="source">#include &lt;OgreSubMesh.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_26

 onmouseover="gutterOver(26)"

><td class="source">#include &lt;OgreSubEntity.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_27

 onmouseover="gutterOver(27)"

><td class="source">#include &lt;OgreMesh.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_28

 onmouseover="gutterOver(28)"

><td class="source">#include &lt;OgreMeshManager.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_29

 onmouseover="gutterOver(29)"

><td class="source">#include &lt;OgreHardwareBufferManager.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_30

 onmouseover="gutterOver(30)"

><td class="source">#include &lt;OgreHardwareBuffer.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_31

 onmouseover="gutterOver(31)"

><td class="source">#include &lt;OgreMaterialManager.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_32

 onmouseover="gutterOver(32)"

><td class="source">#include &lt;OgreMaterial.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_33

 onmouseover="gutterOver(33)"

><td class="source">#include &lt;string&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_34

 onmouseover="gutterOver(34)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_35

 onmouseover="gutterOver(35)"

><td class="source">#include &quot;BatchedGeometry.h&quot;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_36

 onmouseover="gutterOver(36)"

><td class="source">#include &quot;PagedGeometry.h&quot;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_37

 onmouseover="gutterOver(37)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_38

 onmouseover="gutterOver(38)"

><td class="source">using namespace Ogre;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_39

 onmouseover="gutterOver(39)"

><td class="source">using namespace Forests;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_40

 onmouseover="gutterOver(40)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_41

 onmouseover="gutterOver(41)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_42

 onmouseover="gutterOver(42)"

><td class="source">/// For Ogre 1.7.2 and 1.7.3 VertexElementType enum writed as<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_43

 onmouseover="gutterOver(43)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_44

 onmouseover="gutterOver(44)"

><td class="source">/// VET_FLOAT1 = 0<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_45

 onmouseover="gutterOver(45)"

><td class="source">/// VET_FLOAT2 = 1<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_46

 onmouseover="gutterOver(46)"

><td class="source">/// VET_FLOAT3 = 2<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_47

 onmouseover="gutterOver(47)"

><td class="source">/// VET_FLOAT4 = 3<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_48

 onmouseover="gutterOver(48)"

><td class="source">/// VET_COLOUR = 4<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_49

 onmouseover="gutterOver(49)"

><td class="source">/// VET_SHORT1 = 5<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_50

 onmouseover="gutterOver(50)"

><td class="source">/// VET_SHORT2 = 6<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_51

 onmouseover="gutterOver(51)"

><td class="source">/// VET_SHORT3 = 7<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_52

 onmouseover="gutterOver(52)"

><td class="source">/// VET_SHORT4 = 8<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_53

 onmouseover="gutterOver(53)"

><td class="source">/// VET_UBYTE4 = 9<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_54

 onmouseover="gutterOver(54)"

><td class="source">/// VET_COLOUR_ARGB = 10<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_55

 onmouseover="gutterOver(55)"

><td class="source">/// VET_COLOUR_ABGR = 11<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_56

 onmouseover="gutterOver(56)"

><td class="source">const size_t BatchedGeometry::s_vertexType2Size[VET_COLOUR_ABGR + 1] = {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_57

 onmouseover="gutterOver(57)"

><td class="source">   VertexElement::getTypeSize(VET_FLOAT1),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_58

 onmouseover="gutterOver(58)"

><td class="source">   VertexElement::getTypeSize(VET_FLOAT2),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_59

 onmouseover="gutterOver(59)"

><td class="source">   VertexElement::getTypeSize(VET_FLOAT3),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_60

 onmouseover="gutterOver(60)"

><td class="source">   VertexElement::getTypeSize(VET_FLOAT4),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_61

 onmouseover="gutterOver(61)"

><td class="source">   VertexElement::getTypeSize(VET_COLOUR),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_62

 onmouseover="gutterOver(62)"

><td class="source">   VertexElement::getTypeSize(VET_SHORT1),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_63

 onmouseover="gutterOver(63)"

><td class="source">   VertexElement::getTypeSize(VET_SHORT2),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_64

 onmouseover="gutterOver(64)"

><td class="source">   VertexElement::getTypeSize(VET_SHORT3),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_65

 onmouseover="gutterOver(65)"

><td class="source">   VertexElement::getTypeSize(VET_SHORT4),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_66

 onmouseover="gutterOver(66)"

><td class="source">   VertexElement::getTypeSize(VET_UBYTE4),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_67

 onmouseover="gutterOver(67)"

><td class="source">   VertexElement::getTypeSize(VET_COLOUR_ARGB),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_68

 onmouseover="gutterOver(68)"

><td class="source">   VertexElement::getTypeSize(VET_COLOUR_ABGR)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_69

 onmouseover="gutterOver(69)"

><td class="source">};<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_70

 onmouseover="gutterOver(70)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_71

 onmouseover="gutterOver(71)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_72

 onmouseover="gutterOver(72)"

><td class="source">//-------------------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_73

 onmouseover="gutterOver(73)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_74

 onmouseover="gutterOver(74)"

><td class="source">BatchedGeometry::BatchedGeometry(Ogre::SceneManager *mgr, Ogre::SceneNode *rootSceneNode) :<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_75

 onmouseover="gutterOver(75)"

><td class="source">m_fRadius            (0.f),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_76

 onmouseover="gutterOver(76)"

><td class="source">m_fMinDistanceSquared(0.f),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_77

 onmouseover="gutterOver(77)"

><td class="source">m_pSceneMgr          (mgr),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_78

 onmouseover="gutterOver(78)"

><td class="source">m_pSceneNode         (NULL),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_79

 onmouseover="gutterOver(79)"

><td class="source">m_pParentSceneNode   (rootSceneNode),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_80

 onmouseover="gutterOver(80)"

><td class="source">m_bWithinFarDistance (false),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_81

 onmouseover="gutterOver(81)"

><td class="source">m_Built              (false),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_82

 onmouseover="gutterOver(82)"

><td class="source">m_vecCenter          (Ogre::Vector3::ZERO),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_83

 onmouseover="gutterOver(83)"

><td class="source">m_BoundsUndefined    (true)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_84

 onmouseover="gutterOver(84)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_85

 onmouseover="gutterOver(85)"

><td class="source">   assert(rootSceneNode);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_86

 onmouseover="gutterOver(86)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_87

 onmouseover="gutterOver(87)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_88

 onmouseover="gutterOver(88)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_89

 onmouseover="gutterOver(89)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_90

 onmouseover="gutterOver(90)"

><td class="source">BatchedGeometry::~BatchedGeometry()<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_91

 onmouseover="gutterOver(91)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_92

 onmouseover="gutterOver(92)"

><td class="source">   clear();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_93

 onmouseover="gutterOver(93)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_94

 onmouseover="gutterOver(94)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_95

 onmouseover="gutterOver(95)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_96

 onmouseover="gutterOver(96)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_97

 onmouseover="gutterOver(97)"

><td class="source">const Ogre::String&amp; BatchedGeometry::getMovableType() const<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_98

 onmouseover="gutterOver(98)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_99

 onmouseover="gutterOver(99)"

><td class="source">   static const Ogre::String strType = &quot;BatchedGeometry&quot;;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_100

 onmouseover="gutterOver(100)"

><td class="source">   return strType;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_101

 onmouseover="gutterOver(101)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_102

 onmouseover="gutterOver(102)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_103

 onmouseover="gutterOver(103)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_104

 onmouseover="gutterOver(104)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_105

 onmouseover="gutterOver(105)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_106

 onmouseover="gutterOver(106)"

><td class="source">void BatchedGeometry::addEntity(Entity *ent, const Vector3 &amp;position,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_107

 onmouseover="gutterOver(107)"

><td class="source">                                const Quaternion &amp;orientation,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_108

 onmouseover="gutterOver(108)"

><td class="source">                                const Vector3 &amp;scale,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_109

 onmouseover="gutterOver(109)"

><td class="source">                                const Ogre::ColourValue &amp;color)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_110

 onmouseover="gutterOver(110)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_111

 onmouseover="gutterOver(111)"

><td class="source">   const MeshPtr &amp;mesh = ent-&gt;getMesh();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_112

 onmouseover="gutterOver(112)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_113

 onmouseover="gutterOver(113)"

><td class="source">   //If shared vertex data is used, extract into non-shared data<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_114

 onmouseover="gutterOver(114)"

><td class="source">   extractVertexDataFromShared(mesh);	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_115

 onmouseover="gutterOver(115)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_116

 onmouseover="gutterOver(116)"

><td class="source">   //For each subentity<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_117

 onmouseover="gutterOver(117)"

><td class="source">   for (uint32 i = 0; i &lt; ent-&gt;getNumSubEntities(); ++i)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_118

 onmouseover="gutterOver(118)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_119

 onmouseover="gutterOver(119)"

><td class="source">      //Get the subentity<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_120

 onmouseover="gutterOver(120)"

><td class="source">      SubEntity *subEntity = ent-&gt;getSubEntity(i);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_121

 onmouseover="gutterOver(121)"

><td class="source">      SubMesh *subMesh = subEntity-&gt;getSubMesh();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_122

 onmouseover="gutterOver(122)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_123

 onmouseover="gutterOver(123)"

><td class="source">      //Generate a format string that uniquely identifies this material &amp; vertex/index format<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_124

 onmouseover="gutterOver(124)"

><td class="source">      if (subMesh-&gt;vertexData == NULL)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_125

 onmouseover="gutterOver(125)"

><td class="source">         OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, &quot;SubMesh vertex data not found!&quot;, &quot;BatchedGeometry::addEntity()&quot;);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_126

 onmouseover="gutterOver(126)"

><td class="source">      String formatStr = getFormatString(subEntity);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_127

 onmouseover="gutterOver(127)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_128

 onmouseover="gutterOver(128)"

><td class="source">      //If a batch using an identical format exists...<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_129

 onmouseover="gutterOver(129)"

><td class="source">      SubBatch *batch = 0;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_130

 onmouseover="gutterOver(130)"

><td class="source">      TSubBatchMap::iterator batchIter = m_mapSubBatch.find(formatStr);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_131

 onmouseover="gutterOver(131)"

><td class="source">      if (batchIter != m_mapSubBatch.end())<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_132

 onmouseover="gutterOver(132)"

><td class="source">         batch = batchIter-&gt;second; //Use the batch<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_133

 onmouseover="gutterOver(133)"

><td class="source">      else  // Otherwise create a new batch<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_134

 onmouseover="gutterOver(134)"

><td class="source">      {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_135

 onmouseover="gutterOver(135)"

><td class="source">         batch = new SubBatch(this, subEntity);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_136

 onmouseover="gutterOver(136)"

><td class="source">         m_mapSubBatch.insert(std::pair&lt;String, SubBatch*&gt;(formatStr, batch));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_137

 onmouseover="gutterOver(137)"

><td class="source">      }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_138

 onmouseover="gutterOver(138)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_139

 onmouseover="gutterOver(139)"

><td class="source">      //Now add the submesh to the compatible batch<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_140

 onmouseover="gutterOver(140)"

><td class="source">      batch-&gt;addSubEntity(subEntity, position, orientation, scale, color);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_141

 onmouseover="gutterOver(141)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_142

 onmouseover="gutterOver(142)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_143

 onmouseover="gutterOver(143)"

><td class="source">   //Update bounding box<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_144

 onmouseover="gutterOver(144)"

><td class="source">   Matrix4 mat(orientation);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_145

 onmouseover="gutterOver(145)"

><td class="source">   mat.setScale(scale);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_146

 onmouseover="gutterOver(146)"

><td class="source">   AxisAlignedBox entBounds = ent-&gt;getBoundingBox();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_147

 onmouseover="gutterOver(147)"

><td class="source">   entBounds.transform(mat);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_148

 onmouseover="gutterOver(148)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_149

 onmouseover="gutterOver(149)"

><td class="source">   if (m_BoundsUndefined)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_150

 onmouseover="gutterOver(150)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_151

 onmouseover="gutterOver(151)"

><td class="source">      m_boundsAAB.setMinimum(entBounds.getMinimum() + position);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_152

 onmouseover="gutterOver(152)"

><td class="source">      m_boundsAAB.setMaximum(entBounds.getMaximum() + position);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_153

 onmouseover="gutterOver(153)"

><td class="source">      m_BoundsUndefined = false;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_154

 onmouseover="gutterOver(154)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_155

 onmouseover="gutterOver(155)"

><td class="source">   else<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_156

 onmouseover="gutterOver(156)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_157

 onmouseover="gutterOver(157)"

><td class="source">      Vector3 min = m_boundsAAB.getMinimum();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_158

 onmouseover="gutterOver(158)"

><td class="source">      Vector3 max = m_boundsAAB.getMaximum();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_159

 onmouseover="gutterOver(159)"

><td class="source">      min.makeFloor(entBounds.getMinimum() + position);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_160

 onmouseover="gutterOver(160)"

><td class="source">      max.makeCeil(entBounds.getMaximum() + position);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_161

 onmouseover="gutterOver(161)"

><td class="source">      m_boundsAAB.setMinimum(min);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_162

 onmouseover="gutterOver(162)"

><td class="source">      m_boundsAAB.setMaximum(max);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_163

 onmouseover="gutterOver(163)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_164

 onmouseover="gutterOver(164)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_165

 onmouseover="gutterOver(165)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_166

 onmouseover="gutterOver(166)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_167

 onmouseover="gutterOver(167)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_168

 onmouseover="gutterOver(168)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_169

 onmouseover="gutterOver(169)"

><td class="source">uint32 CountUsedVertices(IndexData *id, std::map&lt;uint32, uint32&gt; &amp;ibmap)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_170

 onmouseover="gutterOver(170)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_171

 onmouseover="gutterOver(171)"

><td class="source">   uint32 i, count;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_172

 onmouseover="gutterOver(172)"

><td class="source">   switch (id-&gt;indexBuffer-&gt;getType()) {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_173

 onmouseover="gutterOver(173)"

><td class="source">      case HardwareIndexBuffer::IT_16BIT:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_174

 onmouseover="gutterOver(174)"

><td class="source">         {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_175

 onmouseover="gutterOver(175)"

><td class="source">            uint16 *data = (uint16*)id-&gt;indexBuffer-&gt;lock(id-&gt;indexStart * sizeof(uint16), <br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_176

 onmouseover="gutterOver(176)"

><td class="source">               id-&gt;indexCount * sizeof(uint16), HardwareBuffer::HBL_READ_ONLY);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_177

 onmouseover="gutterOver(177)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_178

 onmouseover="gutterOver(178)"

><td class="source">            for (i = 0; i &lt; id-&gt;indexCount; i++) {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_179

 onmouseover="gutterOver(179)"

><td class="source">               uint16 index = data[i];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_180

 onmouseover="gutterOver(180)"

><td class="source">               if (ibmap.find(index) == ibmap.end()) ibmap[index] = (uint32)(ibmap.size());<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_181

 onmouseover="gutterOver(181)"

><td class="source">            }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_182

 onmouseover="gutterOver(182)"

><td class="source">            count = (uint32)ibmap.size();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_183

 onmouseover="gutterOver(183)"

><td class="source">            id-&gt;indexBuffer-&gt;unlock();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_184

 onmouseover="gutterOver(184)"

><td class="source">         }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_185

 onmouseover="gutterOver(185)"

><td class="source">         break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_186

 onmouseover="gutterOver(186)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_187

 onmouseover="gutterOver(187)"

><td class="source">      case HardwareIndexBuffer::IT_32BIT:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_188

 onmouseover="gutterOver(188)"

><td class="source">         {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_189

 onmouseover="gutterOver(189)"

><td class="source">            uint32 *data = (uint32*)id-&gt;indexBuffer-&gt;lock(id-&gt;indexStart * sizeof(uint32), <br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_190

 onmouseover="gutterOver(190)"

><td class="source">               id-&gt;indexCount * sizeof(uint32), HardwareBuffer::HBL_READ_ONLY);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_191

 onmouseover="gutterOver(191)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_192

 onmouseover="gutterOver(192)"

><td class="source">            for (i = 0; i &lt; id-&gt;indexCount; i++) {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_193

 onmouseover="gutterOver(193)"

><td class="source">               uint32 index = data[i];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_194

 onmouseover="gutterOver(194)"

><td class="source">               if (ibmap.find(index) == ibmap.end()) ibmap[index] = (uint32)(ibmap.size());<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_195

 onmouseover="gutterOver(195)"

><td class="source">            }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_196

 onmouseover="gutterOver(196)"

><td class="source">            count = (uint32)ibmap.size();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_197

 onmouseover="gutterOver(197)"

><td class="source">            id-&gt;indexBuffer-&gt;unlock();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_198

 onmouseover="gutterOver(198)"

><td class="source">         }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_199

 onmouseover="gutterOver(199)"

><td class="source">         break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_200

 onmouseover="gutterOver(200)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_201

 onmouseover="gutterOver(201)"

><td class="source">      default:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_202

 onmouseover="gutterOver(202)"

><td class="source">         throw new Ogre::Exception(0, &quot;Unknown index buffer type&quot;, &quot;Converter.cpp::CountVertices&quot;);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_203

 onmouseover="gutterOver(203)"

><td class="source">         break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_204

 onmouseover="gutterOver(204)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_205

 onmouseover="gutterOver(205)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_206

 onmouseover="gutterOver(206)"

><td class="source">   return count;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_207

 onmouseover="gutterOver(207)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_208

 onmouseover="gutterOver(208)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_209

 onmouseover="gutterOver(209)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_210

 onmouseover="gutterOver(210)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_211

 onmouseover="gutterOver(211)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_212

 onmouseover="gutterOver(212)"

><td class="source">void BatchedGeometry::extractVertexDataFromShared(const Ogre::MeshPtr &amp;mesh)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_213

 onmouseover="gutterOver(213)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_214

 onmouseover="gutterOver(214)"

><td class="source">   if (mesh.isNull() || !mesh-&gt;sharedVertexData)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_215

 onmouseover="gutterOver(215)"

><td class="source">      return;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_216

 onmouseover="gutterOver(216)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_217

 onmouseover="gutterOver(217)"

><td class="source">   Mesh::SubMeshIterator subMeshIterator = mesh-&gt;getSubMeshIterator();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_218

 onmouseover="gutterOver(218)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_219

 onmouseover="gutterOver(219)"

><td class="source">   // Get shared vertex data<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_220

 onmouseover="gutterOver(220)"

><td class="source">   VertexData *oldVertexData = mesh-&gt;sharedVertexData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_221

 onmouseover="gutterOver(221)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_222

 onmouseover="gutterOver(222)"

><td class="source">   while (subMeshIterator.hasMoreElements()) {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_223

 onmouseover="gutterOver(223)"

><td class="source">      SubMesh *subMesh = subMeshIterator.getNext();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_224

 onmouseover="gutterOver(224)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_225

 onmouseover="gutterOver(225)"

><td class="source">      // Get index data<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_226

 onmouseover="gutterOver(226)"

><td class="source">      IndexData *indexData = subMesh-&gt;indexData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_227

 onmouseover="gutterOver(227)"

><td class="source">      HardwareIndexBufferSharedPtr ib = indexData-&gt;indexBuffer;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_228

 onmouseover="gutterOver(228)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_229

 onmouseover="gutterOver(229)"

><td class="source">      // Create new nonshared vertex data<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_230

 onmouseover="gutterOver(230)"

><td class="source">      std::map&lt;uint32, uint32&gt; indicesMap;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_231

 onmouseover="gutterOver(231)"

><td class="source">      VertexData *newVertexData = new VertexData();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_232

 onmouseover="gutterOver(232)"

><td class="source">      newVertexData-&gt;vertexCount = CountUsedVertices(indexData, indicesMap);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_233

 onmouseover="gutterOver(233)"

><td class="source">      //delete newVertexData-&gt;vertexDeclaration;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_234

 onmouseover="gutterOver(234)"

><td class="source">      newVertexData-&gt;vertexDeclaration = oldVertexData-&gt;vertexDeclaration-&gt;clone();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_235

 onmouseover="gutterOver(235)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_236

 onmouseover="gutterOver(236)"

><td class="source">      // Create new vertex buffers<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_237

 onmouseover="gutterOver(237)"

><td class="source">      uint32 buffersCount = (uint32)oldVertexData-&gt;vertexBufferBinding-&gt;getBufferCount();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_238

 onmouseover="gutterOver(238)"

><td class="source">      for (uint32 bufferIndex = 0; bufferIndex &lt; buffersCount; bufferIndex++) {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_239

 onmouseover="gutterOver(239)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_240

 onmouseover="gutterOver(240)"

><td class="source">         // Lock shared vertex buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_241

 onmouseover="gutterOver(241)"

><td class="source">         HardwareVertexBufferSharedPtr oldVertexBuffer = oldVertexData-&gt;vertexBufferBinding-&gt;getBuffer(bufferIndex);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_242

 onmouseover="gutterOver(242)"

><td class="source">         size_t vertexSize = oldVertexBuffer-&gt;getVertexSize();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_243

 onmouseover="gutterOver(243)"

><td class="source">         uint8 *oldLock = (uint8*)oldVertexBuffer-&gt;lock(0, oldVertexData-&gt;vertexCount * vertexSize, HardwareBuffer::HBL_READ_ONLY);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_244

 onmouseover="gutterOver(244)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_245

 onmouseover="gutterOver(245)"

><td class="source">         // Create and lock nonshared vertex buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_246

 onmouseover="gutterOver(246)"

><td class="source">         HardwareVertexBufferSharedPtr newVertexBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_247

 onmouseover="gutterOver(247)"

><td class="source">            vertexSize, newVertexData-&gt;vertexCount, oldVertexBuffer-&gt;getUsage(), oldVertexBuffer-&gt;hasShadowBuffer());<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_248

 onmouseover="gutterOver(248)"

><td class="source">         uint8 *newLock = (uint8*)newVertexBuffer-&gt;lock(0, newVertexData-&gt;vertexCount * vertexSize, HardwareBuffer::HBL_NORMAL);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_249

 onmouseover="gutterOver(249)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_250

 onmouseover="gutterOver(250)"

><td class="source">         // Copy vertices from shared vertex buffer into nonshared vertex buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_251

 onmouseover="gutterOver(251)"

><td class="source">         std::map&lt;uint32, uint32&gt;::iterator i, iend = indicesMap.end();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_252

 onmouseover="gutterOver(252)"

><td class="source">         for (i = indicesMap.begin(); i != iend; i++) {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_253

 onmouseover="gutterOver(253)"

><td class="source">            memcpy(newLock + vertexSize * i-&gt;second, oldLock + vertexSize * i-&gt;first, vertexSize);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_254

 onmouseover="gutterOver(254)"

><td class="source">         }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_255

 onmouseover="gutterOver(255)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_256

 onmouseover="gutterOver(256)"

><td class="source">         // Unlock vertex buffers<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_257

 onmouseover="gutterOver(257)"

><td class="source">         oldVertexBuffer-&gt;unlock();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_258

 onmouseover="gutterOver(258)"

><td class="source">         newVertexBuffer-&gt;unlock();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_259

 onmouseover="gutterOver(259)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_260

 onmouseover="gutterOver(260)"

><td class="source">         // Bind new vertex buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_261

 onmouseover="gutterOver(261)"

><td class="source">         newVertexData-&gt;vertexBufferBinding-&gt;setBinding(bufferIndex, newVertexBuffer);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_262

 onmouseover="gutterOver(262)"

><td class="source">      }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_263

 onmouseover="gutterOver(263)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_264

 onmouseover="gutterOver(264)"

><td class="source">      // Re-create index buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_265

 onmouseover="gutterOver(265)"

><td class="source">      switch (indexData-&gt;indexBuffer-&gt;getType()) {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_266

 onmouseover="gutterOver(266)"

><td class="source">         case HardwareIndexBuffer::IT_16BIT:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_267

 onmouseover="gutterOver(267)"

><td class="source">            {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_268

 onmouseover="gutterOver(268)"

><td class="source">               uint16 *data = (uint16*)indexData-&gt;indexBuffer-&gt;lock(indexData-&gt;indexStart * sizeof(uint16), <br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_269

 onmouseover="gutterOver(269)"

><td class="source">                  indexData-&gt;indexCount * sizeof(uint16), HardwareBuffer::HBL_NORMAL);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_270

 onmouseover="gutterOver(270)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_271

 onmouseover="gutterOver(271)"

><td class="source">               for (uint32 i = 0; i &lt; indexData-&gt;indexCount; i++) {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_272

 onmouseover="gutterOver(272)"

><td class="source">                  data[i] = (uint16)indicesMap[data[i]];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_273

 onmouseover="gutterOver(273)"

><td class="source">               }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_274

 onmouseover="gutterOver(274)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_275

 onmouseover="gutterOver(275)"

><td class="source">               indexData-&gt;indexBuffer-&gt;unlock();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_276

 onmouseover="gutterOver(276)"

><td class="source">            }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_277

 onmouseover="gutterOver(277)"

><td class="source">            break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_278

 onmouseover="gutterOver(278)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_279

 onmouseover="gutterOver(279)"

><td class="source">         case HardwareIndexBuffer::IT_32BIT:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_280

 onmouseover="gutterOver(280)"

><td class="source">            {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_281

 onmouseover="gutterOver(281)"

><td class="source">               uint32 *data = (uint32*)indexData-&gt;indexBuffer-&gt;lock(indexData-&gt;indexStart * sizeof(uint32), <br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_282

 onmouseover="gutterOver(282)"

><td class="source">                  indexData-&gt;indexCount * sizeof(uint32), HardwareBuffer::HBL_NORMAL);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_283

 onmouseover="gutterOver(283)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_284

 onmouseover="gutterOver(284)"

><td class="source">               for (uint32 i = 0; i &lt; indexData-&gt;indexCount; i++) {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_285

 onmouseover="gutterOver(285)"

><td class="source">                  data[i] = (uint32)indicesMap[data[i]];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_286

 onmouseover="gutterOver(286)"

><td class="source">               }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_287

 onmouseover="gutterOver(287)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_288

 onmouseover="gutterOver(288)"

><td class="source">               indexData-&gt;indexBuffer-&gt;unlock();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_289

 onmouseover="gutterOver(289)"

><td class="source">            }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_290

 onmouseover="gutterOver(290)"

><td class="source">            break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_291

 onmouseover="gutterOver(291)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_292

 onmouseover="gutterOver(292)"

><td class="source">         default:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_293

 onmouseover="gutterOver(293)"

><td class="source">            throw new Ogre::Exception(0, &quot;Unknown index buffer type&quot;, &quot;Converter.cpp::CountVertices&quot;);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_294

 onmouseover="gutterOver(294)"

><td class="source">            break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_295

 onmouseover="gutterOver(295)"

><td class="source">      }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_296

 onmouseover="gutterOver(296)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_297

 onmouseover="gutterOver(297)"

><td class="source">      // Store new attributes<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_298

 onmouseover="gutterOver(298)"

><td class="source">      subMesh-&gt;useSharedVertices = false;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_299

 onmouseover="gutterOver(299)"

><td class="source">      subMesh-&gt;vertexData = newVertexData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_300

 onmouseover="gutterOver(300)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_301

 onmouseover="gutterOver(301)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_302

 onmouseover="gutterOver(302)"

><td class="source">   // Release shared vertex data<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_303

 onmouseover="gutterOver(303)"

><td class="source">   delete mesh-&gt;sharedVertexData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_304

 onmouseover="gutterOver(304)"

><td class="source">   mesh-&gt;sharedVertexData = NULL;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_305

 onmouseover="gutterOver(305)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_306

 onmouseover="gutterOver(306)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_307

 onmouseover="gutterOver(307)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_308

 onmouseover="gutterOver(308)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_309

 onmouseover="gutterOver(309)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_310

 onmouseover="gutterOver(310)"

><td class="source">String BatchedGeometry::getFormatString(SubEntity *ent)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_311

 onmouseover="gutterOver(311)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_312

 onmouseover="gutterOver(312)"

><td class="source">   static char buf[1024];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_313

 onmouseover="gutterOver(313)"

><td class="source">   // add materialname and buffer type<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_314

 onmouseover="gutterOver(314)"

><td class="source">   int countWritten =  sprintf(buf, &quot;%s|%d&quot;, ent-&gt;getMaterialName().c_str(), ent-&gt;getSubMesh()-&gt;indexData-&gt;indexBuffer-&gt;getType());<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_315

 onmouseover="gutterOver(315)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_316

 onmouseover="gutterOver(316)"

><td class="source">   // now add vertex decl<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_317

 onmouseover="gutterOver(317)"

><td class="source">   const VertexDeclaration::VertexElementList &amp;elemList = ent-&gt;getSubMesh()-&gt;vertexData-&gt;vertexDeclaration-&gt;getElements();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_318

 onmouseover="gutterOver(318)"

><td class="source">   for (VertexDeclaration::VertexElementList::const_iterator i = elemList.begin(), iend = elemList.end(); i != iend; ++i)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_319

 onmouseover="gutterOver(319)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_320

 onmouseover="gutterOver(320)"

><td class="source">      const VertexElement &amp;el = *i;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_321

 onmouseover="gutterOver(321)"

><td class="source">      countWritten += sprintf(buf + countWritten, &quot;|%d|%d|%d&quot;, el.getSource(), el.getSemantic(), el.getType());<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_322

 onmouseover="gutterOver(322)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_323

 onmouseover="gutterOver(323)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_324

 onmouseover="gutterOver(324)"

><td class="source">   return buf;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_325

 onmouseover="gutterOver(325)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_326

 onmouseover="gutterOver(326)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_327

 onmouseover="gutterOver(327)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_328

 onmouseover="gutterOver(328)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_329

 onmouseover="gutterOver(329)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_330

 onmouseover="gutterOver(330)"

><td class="source">void BatchedGeometry::build()<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_331

 onmouseover="gutterOver(331)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_332

 onmouseover="gutterOver(332)"

><td class="source">   ///Make sure the batch hasn&#39;t already been built<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_333

 onmouseover="gutterOver(333)"

><td class="source">   if (m_Built)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_334

 onmouseover="gutterOver(334)"

><td class="source">      OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, &quot;Invalid call to build() - geometry is already batched (call clear() first)&quot;, &quot;BatchedGeometry::GeomBatch::build()&quot;);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_335

 onmouseover="gutterOver(335)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_336

 onmouseover="gutterOver(336)"

><td class="source">   if (!m_mapSubBatch.empty())<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_337

 onmouseover="gutterOver(337)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_338

 onmouseover="gutterOver(338)"

><td class="source">      // Finish bounds information<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_339

 onmouseover="gutterOver(339)"

><td class="source">      m_vecCenter = m_boundsAAB.getCenter();                            // Calculate bounds center<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_340

 onmouseover="gutterOver(340)"

><td class="source">      m_boundsAAB.setMinimum(m_boundsAAB.getMinimum() - m_vecCenter);   // Center the bounding box<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_341

 onmouseover="gutterOver(341)"

><td class="source">      m_boundsAAB.setMaximum(m_boundsAAB.getMaximum() - m_vecCenter);	// Center the bounding box<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_342

 onmouseover="gutterOver(342)"

><td class="source">      m_fRadius = m_boundsAAB.getMaximum().length();                    // Calculate BB radius<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_343

 onmouseover="gutterOver(343)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_344

 onmouseover="gutterOver(344)"

><td class="source">      // Create scene node<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_345

 onmouseover="gutterOver(345)"

><td class="source">      m_pSceneNode = m_pParentSceneNode-&gt;createChildSceneNode(m_vecCenter);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_346

 onmouseover="gutterOver(346)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_347

 onmouseover="gutterOver(347)"

><td class="source">      //Build each batch<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_348

 onmouseover="gutterOver(348)"

><td class="source">      for (TSubBatchMap::iterator i = m_mapSubBatch.begin(), iend = m_mapSubBatch.end(); i != iend; ++i)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_349

 onmouseover="gutterOver(349)"

><td class="source">         i-&gt;second-&gt;build();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_350

 onmouseover="gutterOver(350)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_351

 onmouseover="gutterOver(351)"

><td class="source">      m_pSceneNode-&gt;attachObject(this);   // Attach the batch to the scene node<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_352

 onmouseover="gutterOver(352)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_353

 onmouseover="gutterOver(353)"

><td class="source">      //Debug<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_354

 onmouseover="gutterOver(354)"

><td class="source">      //sceneNode-&gt;showBoundingBox(true);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_355

 onmouseover="gutterOver(355)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_356

 onmouseover="gutterOver(356)"

><td class="source">      m_Built = true;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_357

 onmouseover="gutterOver(357)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_358

 onmouseover="gutterOver(358)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_359

 onmouseover="gutterOver(359)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_360

 onmouseover="gutterOver(360)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_361

 onmouseover="gutterOver(361)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_362

 onmouseover="gutterOver(362)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_363

 onmouseover="gutterOver(363)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_364

 onmouseover="gutterOver(364)"

><td class="source">void BatchedGeometry::clear()<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_365

 onmouseover="gutterOver(365)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_366

 onmouseover="gutterOver(366)"

><td class="source">   //Remove the batch from the scene<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_367

 onmouseover="gutterOver(367)"

><td class="source">   if (m_pSceneNode)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_368

 onmouseover="gutterOver(368)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_369

 onmouseover="gutterOver(369)"

><td class="source">      m_pSceneNode-&gt;removeAllChildren();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_370

 onmouseover="gutterOver(370)"

><td class="source">      if (m_pSceneNode-&gt;getParent())<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_371

 onmouseover="gutterOver(371)"

><td class="source">         m_pSceneNode-&gt;getParentSceneNode()-&gt;removeAndDestroyChild(m_pSceneNode-&gt;getName());<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_372

 onmouseover="gutterOver(372)"

><td class="source">      else<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_373

 onmouseover="gutterOver(373)"

><td class="source">         m_pSceneMgr-&gt;destroySceneNode(m_pSceneNode);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_374

 onmouseover="gutterOver(374)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_375

 onmouseover="gutterOver(375)"

><td class="source">      m_pSceneNode = 0;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_376

 onmouseover="gutterOver(376)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_377

 onmouseover="gutterOver(377)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_378

 onmouseover="gutterOver(378)"

><td class="source">   //Reset bounds information<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_379

 onmouseover="gutterOver(379)"

><td class="source">   m_BoundsUndefined = true;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_380

 onmouseover="gutterOver(380)"

><td class="source">   m_vecCenter = Vector3::ZERO;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_381

 onmouseover="gutterOver(381)"

><td class="source">   m_fRadius = 0.f;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_382

 onmouseover="gutterOver(382)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_383

 onmouseover="gutterOver(383)"

><td class="source">   //Delete each batch<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_384

 onmouseover="gutterOver(384)"

><td class="source">   for (TSubBatchMap::iterator i = m_mapSubBatch.begin(), iend = m_mapSubBatch.end(); i != iend; ++i)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_385

 onmouseover="gutterOver(385)"

><td class="source">      delete i-&gt;second;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_386

 onmouseover="gutterOver(386)"

><td class="source">   m_mapSubBatch.clear();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_387

 onmouseover="gutterOver(387)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_388

 onmouseover="gutterOver(388)"

><td class="source">   m_Built = false;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_389

 onmouseover="gutterOver(389)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_390

 onmouseover="gutterOver(390)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_391

 onmouseover="gutterOver(391)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_392

 onmouseover="gutterOver(392)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_393

 onmouseover="gutterOver(393)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_394

 onmouseover="gutterOver(394)"

><td class="source">void BatchedGeometry::_updateRenderQueue(RenderQueue *queue)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_395

 onmouseover="gutterOver(395)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_396

 onmouseover="gutterOver(396)"

><td class="source">   assert(isVisible() &amp;&amp; &quot;Ogre core code must detect that this MovableObject invisible&quot;);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_397

 onmouseover="gutterOver(397)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_398

 onmouseover="gutterOver(398)"

><td class="source">   // SVA speed up adding<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_399

 onmouseover="gutterOver(399)"

><td class="source">   Ogre::RenderQueueGroup *rqg = queue-&gt;getQueueGroup(getRenderQueueGroup());<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_400

 onmouseover="gutterOver(400)"

><td class="source">   for (TSubBatchMap::const_iterator i = m_mapSubBatch.begin(), iend = m_mapSubBatch.end(); i != iend; ++i)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_401

 onmouseover="gutterOver(401)"

><td class="source">      i-&gt;second-&gt;addSelfToRenderQueue(rqg);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_402

 onmouseover="gutterOver(402)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_403

 onmouseover="gutterOver(403)"

><td class="source">   ////If visible...<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_404

 onmouseover="gutterOver(404)"

><td class="source">   //if (isVisible()){<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_405

 onmouseover="gutterOver(405)"

><td class="source">   //   //Ask each batch to add itself to the render queue if appropriate<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_406

 onmouseover="gutterOver(406)"

><td class="source">   //   for (SubBatchMap::iterator i = subBatchMap.begin(); i != subBatchMap.end(); ++i){<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_407

 onmouseover="gutterOver(407)"

><td class="source">   //      i-&gt;second-&gt;addSelfToRenderQueue(queue, getRenderQueueGroup());<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_408

 onmouseover="gutterOver(408)"

><td class="source">   //   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_409

 onmouseover="gutterOver(409)"

><td class="source">   //}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_410

 onmouseover="gutterOver(410)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_411

 onmouseover="gutterOver(411)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_412

 onmouseover="gutterOver(412)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_413

 onmouseover="gutterOver(413)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_414

 onmouseover="gutterOver(414)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_415

 onmouseover="gutterOver(415)"

><td class="source">bool BatchedGeometry::isVisible()<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_416

 onmouseover="gutterOver(416)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_417

 onmouseover="gutterOver(417)"

><td class="source">   return mVisible &amp;&amp; m_bWithinFarDistance;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_418

 onmouseover="gutterOver(418)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_419

 onmouseover="gutterOver(419)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_420

 onmouseover="gutterOver(420)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_421

 onmouseover="gutterOver(421)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_422

 onmouseover="gutterOver(422)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_423

 onmouseover="gutterOver(423)"

><td class="source">void BatchedGeometry::_notifyCurrentCamera(Camera *cam)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_424

 onmouseover="gutterOver(424)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_425

 onmouseover="gutterOver(425)"

><td class="source">   if (getRenderingDistance() == Ogre::Real(0.))<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_426

 onmouseover="gutterOver(426)"

><td class="source">      m_bWithinFarDistance = true;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_427

 onmouseover="gutterOver(427)"

><td class="source">   else<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_428

 onmouseover="gutterOver(428)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_429

 onmouseover="gutterOver(429)"

><td class="source">      //Calculate camera distance<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_430

 onmouseover="gutterOver(430)"

><td class="source">      Vector3 camVec = _convertToLocal(cam-&gt;getDerivedPosition()) - m_vecCenter;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_431

 onmouseover="gutterOver(431)"

><td class="source">      Real centerDistanceSquared = camVec.squaredLength();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_432

 onmouseover="gutterOver(432)"

><td class="source">      m_fMinDistanceSquared = std::max(Ogre::Real(0.), centerDistanceSquared - (m_fRadius * m_fRadius));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_433

 onmouseover="gutterOver(433)"

><td class="source">      //Note: centerDistanceSquared measures the distance between the camera and the center of the GeomBatch,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_434

 onmouseover="gutterOver(434)"

><td class="source">      //while minDistanceSquared measures the closest distance between the camera and the closest edge of the<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_435

 onmouseover="gutterOver(435)"

><td class="source">      //geometry&#39;s bounding sphere.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_436

 onmouseover="gutterOver(436)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_437

 onmouseover="gutterOver(437)"

><td class="source">      //Determine whether the BatchedGeometry is within the far rendering distance<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_438

 onmouseover="gutterOver(438)"

><td class="source">      m_bWithinFarDistance = m_fMinDistanceSquared &lt;= Math::Sqr(getRenderingDistance());<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_439

 onmouseover="gutterOver(439)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_440

 onmouseover="gutterOver(440)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_441

 onmouseover="gutterOver(441)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_442

 onmouseover="gutterOver(442)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_443

 onmouseover="gutterOver(443)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_444

 onmouseover="gutterOver(444)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_445

 onmouseover="gutterOver(445)"

><td class="source">Ogre::Vector3 BatchedGeometry::_convertToLocal(const Vector3 &amp;globalVec) const<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_446

 onmouseover="gutterOver(446)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_447

 onmouseover="gutterOver(447)"

><td class="source">   //Convert from the given global position to the local coordinate system of the parent scene node.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_448

 onmouseover="gutterOver(448)"

><td class="source">   return (m_pParentSceneNode-&gt;getOrientation().Inverse() * globalVec);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_449

 onmouseover="gutterOver(449)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_450

 onmouseover="gutterOver(450)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_451

 onmouseover="gutterOver(451)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_452

 onmouseover="gutterOver(452)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_453

 onmouseover="gutterOver(453)"

><td class="source">//=============================================================================<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_454

 onmouseover="gutterOver(454)"

><td class="source">// BatchedGeometry::SubBatch implementation<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_455

 onmouseover="gutterOver(455)"

><td class="source">//=============================================================================<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_456

 onmouseover="gutterOver(456)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_457

 onmouseover="gutterOver(457)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_458

 onmouseover="gutterOver(458)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_459

 onmouseover="gutterOver(459)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_460

 onmouseover="gutterOver(460)"

><td class="source">BatchedGeometry::SubBatch::SubBatch(BatchedGeometry *parent, SubEntity *ent) :<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_461

 onmouseover="gutterOver(461)"

><td class="source">m_pBestTechnique        (NULL),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_462

 onmouseover="gutterOver(462)"

><td class="source">m_pVertexData           (0),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_463

 onmouseover="gutterOver(463)"

><td class="source">m_pIndexData            (0),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_464

 onmouseover="gutterOver(464)"

><td class="source">m_Built                 (false),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_465

 onmouseover="gutterOver(465)"

><td class="source">m_RequireVertexColors   (false),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_466

 onmouseover="gutterOver(466)"

><td class="source">m_pSubMesh              (0),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_467

 onmouseover="gutterOver(467)"

><td class="source">m_pParentGeom           (parent)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_468

 onmouseover="gutterOver(468)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_469

 onmouseover="gutterOver(469)"

><td class="source">   assert(ent);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_470

 onmouseover="gutterOver(470)"

><td class="source">   m_pSubMesh = ent-&gt;getSubMesh();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_471

 onmouseover="gutterOver(471)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_472

 onmouseover="gutterOver(472)"

><td class="source">   const Ogre::MaterialPtr &amp;parentMaterial = ent-&gt;getMaterial();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_473

 onmouseover="gutterOver(473)"

><td class="source">   if (parentMaterial.isNull())<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_474

 onmouseover="gutterOver(474)"

><td class="source">      OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, &quot;BatchedGeometry. Empty parent material&quot;, &quot;BatchedGeometry::SubBatch::SubBatch&quot;);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_475

 onmouseover="gutterOver(475)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_476

 onmouseover="gutterOver(476)"

><td class="source">   // SVA clone material<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_477

 onmouseover="gutterOver(477)"

><td class="source">   // This function is used to make a single clone of materials used, since the materials<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_478

 onmouseover="gutterOver(478)"

><td class="source">   // will be modified by the batch system (and it wouldn&#39;t be good to modify the original materials<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_479

 onmouseover="gutterOver(479)"

><td class="source">   // that the user may be using somewhere else).<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_480

 onmouseover="gutterOver(480)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_481

 onmouseover="gutterOver(481)"

><td class="source">      Ogre::String newName = parentMaterial-&gt;getName() + &quot;_Batched&quot;;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_482

 onmouseover="gutterOver(482)"

><td class="source">      m_ptrMaterial = MaterialManager::getSingleton().getByName(newName, parentMaterial-&gt;getGroup());<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_483

 onmouseover="gutterOver(483)"

><td class="source">      if (m_ptrMaterial.isNull())<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_484

 onmouseover="gutterOver(484)"

><td class="source">         m_ptrMaterial = parentMaterial-&gt;clone(newName);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_485

 onmouseover="gutterOver(485)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_486

 onmouseover="gutterOver(486)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_487

 onmouseover="gutterOver(487)"

><td class="source">   //Setup vertex/index data structure<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_488

 onmouseover="gutterOver(488)"

><td class="source">   m_pVertexData = m_pSubMesh-&gt;vertexData-&gt;clone(false);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_489

 onmouseover="gutterOver(489)"

><td class="source">   m_pIndexData = m_pSubMesh-&gt;indexData-&gt;clone(false);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_490

 onmouseover="gutterOver(490)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_491

 onmouseover="gutterOver(491)"

><td class="source">   //Remove blend weights from vertex format<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_492

 onmouseover="gutterOver(492)"

><td class="source">   const VertexElement* blendIndices = m_pVertexData-&gt;vertexDeclaration-&gt;findElementBySemantic(VES_BLEND_INDICES);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_493

 onmouseover="gutterOver(493)"

><td class="source">   const VertexElement* blendWeights = m_pVertexData-&gt;vertexDeclaration-&gt;findElementBySemantic(VES_BLEND_WEIGHTS);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_494

 onmouseover="gutterOver(494)"

><td class="source">   if (blendIndices &amp;&amp; blendWeights)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_495

 onmouseover="gutterOver(495)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_496

 onmouseover="gutterOver(496)"

><td class="source">      //Check for format errors<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_497

 onmouseover="gutterOver(497)"

><td class="source">      assert(blendIndices-&gt;getSource() == blendWeights-&gt;getSource()<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_498

 onmouseover="gutterOver(498)"

><td class="source">         &amp;&amp; &quot;Blend indices and weights should be in the same buffer&quot;);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_499

 onmouseover="gutterOver(499)"

><td class="source">      assert(blendIndices-&gt;getSize() + blendWeights-&gt;getSize() == m_pVertexData-&gt;vertexBufferBinding-&gt;getBuffer(blendIndices-&gt;getSource())-&gt;getVertexSize()<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_500

 onmouseover="gutterOver(500)"

><td class="source">         &amp;&amp; &quot;Blend indices and blend buffers should have buffer to themselves!&quot;);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_501

 onmouseover="gutterOver(501)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_502

 onmouseover="gutterOver(502)"

><td class="source">      //Remove the blend weights<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_503

 onmouseover="gutterOver(503)"

><td class="source">      m_pVertexData-&gt;vertexBufferBinding-&gt;unsetBinding(blendIndices-&gt;getSource());<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_504

 onmouseover="gutterOver(504)"

><td class="source">      m_pVertexData-&gt;vertexDeclaration-&gt;removeElement(VES_BLEND_INDICES);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_505

 onmouseover="gutterOver(505)"

><td class="source">      m_pVertexData-&gt;vertexDeclaration-&gt;removeElement(VES_BLEND_WEIGHTS);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_506

 onmouseover="gutterOver(506)"

><td class="source">      m_pVertexData-&gt;closeGapsInBindings();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_507

 onmouseover="gutterOver(507)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_508

 onmouseover="gutterOver(508)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_509

 onmouseover="gutterOver(509)"

><td class="source">   //Reset vertex/index count<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_510

 onmouseover="gutterOver(510)"

><td class="source">   m_pVertexData-&gt;vertexStart = 0;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_511

 onmouseover="gutterOver(511)"

><td class="source">   m_pVertexData-&gt;vertexCount = 0;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_512

 onmouseover="gutterOver(512)"

><td class="source">   m_pIndexData-&gt;indexStart   = 0;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_513

 onmouseover="gutterOver(513)"

><td class="source">   m_pIndexData-&gt;indexCount   = 0;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_514

 onmouseover="gutterOver(514)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_515

 onmouseover="gutterOver(515)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_516

 onmouseover="gutterOver(516)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_517

 onmouseover="gutterOver(517)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_518

 onmouseover="gutterOver(518)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_519

 onmouseover="gutterOver(519)"

><td class="source">BatchedGeometry::SubBatch::~SubBatch()<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_520

 onmouseover="gutterOver(520)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_521

 onmouseover="gutterOver(521)"

><td class="source">   clear();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_522

 onmouseover="gutterOver(522)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_523

 onmouseover="gutterOver(523)"

><td class="source">   OGRE_DELETE m_pVertexData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_524

 onmouseover="gutterOver(524)"

><td class="source">   OGRE_DELETE m_pIndexData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_525

 onmouseover="gutterOver(525)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_526

 onmouseover="gutterOver(526)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_527

 onmouseover="gutterOver(527)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_528

 onmouseover="gutterOver(528)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_529

 onmouseover="gutterOver(529)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_530

 onmouseover="gutterOver(530)"

><td class="source">void BatchedGeometry::SubBatch::addSubEntity(SubEntity *ent, const Vector3 &amp;position,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_531

 onmouseover="gutterOver(531)"

><td class="source">                                             const Quaternion &amp;orientation, const Vector3 &amp;scale,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_532

 onmouseover="gutterOver(532)"

><td class="source">                                             const Ogre::ColourValue &amp;color, void* userData)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_533

 onmouseover="gutterOver(533)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_534

 onmouseover="gutterOver(534)"

><td class="source">   assert(!m_Built);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_535

 onmouseover="gutterOver(535)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_536

 onmouseover="gutterOver(536)"

><td class="source">   //Add this submesh to the queue<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_537

 onmouseover="gutterOver(537)"

><td class="source">   QueuedMesh newMesh(ent-&gt;getSubMesh(), position, orientation, scale, color, userData);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_538

 onmouseover="gutterOver(538)"

><td class="source">   if (color != ColourValue::White)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_539

 onmouseover="gutterOver(539)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_540

 onmouseover="gutterOver(540)"

><td class="source">      m_RequireVertexColors = true;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_541

 onmouseover="gutterOver(541)"

><td class="source">      VertexElementType format = Root::getSingleton().getRenderSystem()-&gt;getColourVertexElementType();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_542

 onmouseover="gutterOver(542)"

><td class="source">      switch (format)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_543

 onmouseover="gutterOver(543)"

><td class="source">      {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_544

 onmouseover="gutterOver(544)"

><td class="source">      case VET_COLOUR_ARGB:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_545

 onmouseover="gutterOver(545)"

><td class="source">         std::swap(newMesh.color.r, newMesh.color.b);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_546

 onmouseover="gutterOver(546)"

><td class="source">         break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_547

 onmouseover="gutterOver(547)"

><td class="source">      case VET_COLOUR_ABGR:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_548

 onmouseover="gutterOver(548)"

><td class="source">         break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_549

 onmouseover="gutterOver(549)"

><td class="source">      default:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_550

 onmouseover="gutterOver(550)"

><td class="source">         OGRE_EXCEPT(0, &quot;Unknown RenderSystem color format&quot;, &quot;BatchedGeometry::SubBatch::addSubMesh()&quot;);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_551

 onmouseover="gutterOver(551)"

><td class="source">         break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_552

 onmouseover="gutterOver(552)"

><td class="source">      }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_553

 onmouseover="gutterOver(553)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_554

 onmouseover="gutterOver(554)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_555

 onmouseover="gutterOver(555)"

><td class="source">   m_queueMesh.push_back(newMesh);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_556

 onmouseover="gutterOver(556)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_557

 onmouseover="gutterOver(557)"

><td class="source">   //Increment the vertex/index count so the buffers will have room for this mesh<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_558

 onmouseover="gutterOver(558)"

><td class="source">   m_pVertexData-&gt;vertexCount += newMesh.subMesh-&gt;vertexData-&gt;vertexCount;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_559

 onmouseover="gutterOver(559)"

><td class="source">   m_pIndexData-&gt;indexCount   += newMesh.subMesh-&gt;indexData-&gt;indexCount;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_560

 onmouseover="gutterOver(560)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_561

 onmouseover="gutterOver(561)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_562

 onmouseover="gutterOver(562)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_563

 onmouseover="gutterOver(563)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_564

 onmouseover="gutterOver(564)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_565

 onmouseover="gutterOver(565)"

><td class="source">void BatchedGeometry::SubBatch::build()<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_566

 onmouseover="gutterOver(566)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_567

 onmouseover="gutterOver(567)"

><td class="source">   assert(!m_Built);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_568

 onmouseover="gutterOver(568)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_569

 onmouseover="gutterOver(569)"

><td class="source">   HardwareIndexBuffer::IndexType srcIndexType = m_pSubMesh-&gt;indexData-&gt;indexBuffer-&gt;getType();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_570

 onmouseover="gutterOver(570)"

><td class="source">   HardwareIndexBuffer::IndexType destIndexType =                             // type of index buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_571

 onmouseover="gutterOver(571)"

><td class="source">      m_pVertexData-&gt;vertexCount &gt; 0xFFFF || srcIndexType == HardwareIndexBuffer::IT_32BIT ? <br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_572

 onmouseover="gutterOver(572)"

><td class="source">      HardwareIndexBuffer::IT_32BIT : HardwareIndexBuffer::IT_16BIT;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_573

 onmouseover="gutterOver(573)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_574

 onmouseover="gutterOver(574)"

><td class="source">   //Allocate the index buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_575

 onmouseover="gutterOver(575)"

><td class="source">   m_pIndexData-&gt;indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_576

 onmouseover="gutterOver(576)"

><td class="source">      destIndexType, m_pIndexData-&gt;indexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_577

 onmouseover="gutterOver(577)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_578

 onmouseover="gutterOver(578)"

><td class="source">   //Lock the index buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_579

 onmouseover="gutterOver(579)"

><td class="source">   uint32 *indexBuffer32;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_580

 onmouseover="gutterOver(580)"

><td class="source">   uint16 *indexBuffer16;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_581

 onmouseover="gutterOver(581)"

><td class="source">   if (destIndexType == HardwareIndexBuffer::IT_32BIT)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_582

 onmouseover="gutterOver(582)"

><td class="source">      indexBuffer32 = static_cast&lt;uint32*&gt;(m_pIndexData-&gt;indexBuffer-&gt;lock(HardwareBuffer::HBL_DISCARD));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_583

 onmouseover="gutterOver(583)"

><td class="source">   else<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_584

 onmouseover="gutterOver(584)"

><td class="source">      indexBuffer16 = static_cast&lt;uint16*&gt;(m_pIndexData-&gt;indexBuffer-&gt;lock(HardwareBuffer::HBL_DISCARD));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_585

 onmouseover="gutterOver(585)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_586

 onmouseover="gutterOver(586)"

><td class="source">   //Allocate &amp; lock the vertex buffers<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_587

 onmouseover="gutterOver(587)"

><td class="source">   std::vector&lt;uchar*&gt; vertexBuffers;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_588

 onmouseover="gutterOver(588)"

><td class="source">   std::vector&lt;VertexDeclaration::VertexElementList&gt; vertexBufferElements;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_589

 onmouseover="gutterOver(589)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_590

 onmouseover="gutterOver(590)"

><td class="source">   VertexBufferBinding *vertBinding = m_pVertexData-&gt;vertexBufferBinding;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_591

 onmouseover="gutterOver(591)"

><td class="source">   VertexDeclaration *vertDecl = m_pVertexData-&gt;vertexDeclaration;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_592

 onmouseover="gutterOver(592)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_593

 onmouseover="gutterOver(593)"

><td class="source">   for (Ogre::ushort i = 0; i &lt; vertBinding-&gt;getBufferCount(); ++i)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_594

 onmouseover="gutterOver(594)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_595

 onmouseover="gutterOver(595)"

><td class="source">      HardwareVertexBufferSharedPtr buffer = HardwareBufferManager::getSingleton().createVertexBuffer(<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_596

 onmouseover="gutterOver(596)"

><td class="source">         vertDecl-&gt;getVertexSize(i), m_pVertexData-&gt;vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_597

 onmouseover="gutterOver(597)"

><td class="source">      vertBinding-&gt;setBinding(i, buffer);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_598

 onmouseover="gutterOver(598)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_599

 onmouseover="gutterOver(599)"

><td class="source">      vertexBuffers.push_back(static_cast&lt;uchar*&gt;(buffer-&gt;lock(HardwareBuffer::HBL_DISCARD)));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_600

 onmouseover="gutterOver(600)"

><td class="source">      vertexBufferElements.push_back(vertDecl-&gt;findElementsBySource(i));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_601

 onmouseover="gutterOver(601)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_602

 onmouseover="gutterOver(602)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_603

 onmouseover="gutterOver(603)"

><td class="source">   //If no vertex colors are used, make sure the final batch includes them (so the shade values work)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_604

 onmouseover="gutterOver(604)"

><td class="source">   if (m_RequireVertexColors)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_605

 onmouseover="gutterOver(605)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_606

 onmouseover="gutterOver(606)"

><td class="source">      if (!m_pVertexData-&gt;vertexDeclaration-&gt;findElementBySemantic(VES_DIFFUSE))<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_607

 onmouseover="gutterOver(607)"

><td class="source">      {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_608

 onmouseover="gutterOver(608)"

><td class="source">         Ogre::ushort i = (Ogre::ushort)vertBinding-&gt;getBufferCount();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_609

 onmouseover="gutterOver(609)"

><td class="source">         vertDecl-&gt;addElement(i, 0, VET_COLOUR, VES_DIFFUSE);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_610

 onmouseover="gutterOver(610)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_611

 onmouseover="gutterOver(611)"

><td class="source">         HardwareVertexBufferSharedPtr buffer = HardwareBufferManager::getSingleton().createVertexBuffer(<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_612

 onmouseover="gutterOver(612)"

><td class="source">            vertDecl-&gt;getVertexSize(i), m_pVertexData-&gt;vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_613

 onmouseover="gutterOver(613)"

><td class="source">         vertBinding-&gt;setBinding(i, buffer);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_614

 onmouseover="gutterOver(614)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_615

 onmouseover="gutterOver(615)"

><td class="source">         vertexBuffers.push_back(static_cast&lt;uchar*&gt;(buffer-&gt;lock(HardwareBuffer::HBL_DISCARD)));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_616

 onmouseover="gutterOver(616)"

><td class="source">         vertexBufferElements.push_back(vertDecl-&gt;findElementsBySource(i));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_617

 onmouseover="gutterOver(617)"

><td class="source">      }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_618

 onmouseover="gutterOver(618)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_619

 onmouseover="gutterOver(619)"

><td class="source">      m_ptrMaterial-&gt;getTechnique(0)-&gt;getPass(0)-&gt;setVertexColourTracking(TVC_AMBIENT);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_620

 onmouseover="gutterOver(620)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_621

 onmouseover="gutterOver(621)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_622

 onmouseover="gutterOver(622)"

><td class="source">   //For each queued mesh...<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_623

 onmouseover="gutterOver(623)"

><td class="source">   size_t indexOffset = 0;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_624

 onmouseover="gutterOver(624)"

><td class="source">   for (size_t iMesh = 0, cntMeshes = m_queueMesh.size(); iMesh &lt; cntMeshes; ++iMesh)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_625

 onmouseover="gutterOver(625)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_626

 onmouseover="gutterOver(626)"

><td class="source">      const QueuedMesh &amp;queuedMesh = m_queueMesh[iMesh]; // &lt;-- std::vector<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_627

 onmouseover="gutterOver(627)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_628

 onmouseover="gutterOver(628)"

><td class="source">      // If orientation identity we can skip many operation in vertex processing<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_629

 onmouseover="gutterOver(629)"

><td class="source">      if (queuedMesh.orientation == Ogre::Quaternion::IDENTITY)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_630

 onmouseover="gutterOver(630)"

><td class="source">         _buildIdentiryOrientation(queuedMesh, m_pParentGeom-&gt;m_vecCenter, vertexBufferElements, vertexBuffers, m_pVertexData);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_631

 onmouseover="gutterOver(631)"

><td class="source">      else<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_632

 onmouseover="gutterOver(632)"

><td class="source">         _buildFullTransform(queuedMesh, m_pParentGeom-&gt;m_vecCenter, vertexBufferElements, vertexBuffers, m_pVertexData);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_633

 onmouseover="gutterOver(633)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_634

 onmouseover="gutterOver(634)"

><td class="source">      //Copy mesh index data into the index buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_635

 onmouseover="gutterOver(635)"

><td class="source">      Ogre::IndexData *sourceIndexData = queuedMesh.subMesh-&gt;indexData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_636

 onmouseover="gutterOver(636)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_637

 onmouseover="gutterOver(637)"

><td class="source">      if (srcIndexType == HardwareIndexBuffer::IT_32BIT)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_638

 onmouseover="gutterOver(638)"

><td class="source">      {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_639

 onmouseover="gutterOver(639)"

><td class="source">         //Lock the input buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_640

 onmouseover="gutterOver(640)"

><td class="source">         uint32 *source = static_cast&lt;uint32*&gt;(sourceIndexData-&gt;indexBuffer-&gt;lock(<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_641

 onmouseover="gutterOver(641)"

><td class="source">            sourceIndexData-&gt;indexStart, sourceIndexData-&gt;indexCount, HardwareBuffer::HBL_READ_ONLY));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_642

 onmouseover="gutterOver(642)"

><td class="source">         uint32 *sourceEnd = source + sourceIndexData-&gt;indexCount;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_643

 onmouseover="gutterOver(643)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_644

 onmouseover="gutterOver(644)"

><td class="source">         //And copy it to the output buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_645

 onmouseover="gutterOver(645)"

><td class="source">         while (source != sourceEnd)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_646

 onmouseover="gutterOver(646)"

><td class="source">            *indexBuffer32++ = static_cast&lt;uint32&gt;(*source++ + indexOffset);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_647

 onmouseover="gutterOver(647)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_648

 onmouseover="gutterOver(648)"

><td class="source">         sourceIndexData-&gt;indexBuffer-&gt;unlock();                     // Unlock the input buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_649

 onmouseover="gutterOver(649)"

><td class="source">         indexOffset += queuedMesh.subMesh-&gt;vertexData-&gt;vertexCount; // Increment the index offset<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_650

 onmouseover="gutterOver(650)"

><td class="source">      }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_651

 onmouseover="gutterOver(651)"

><td class="source">      else<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_652

 onmouseover="gutterOver(652)"

><td class="source">      {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_653

 onmouseover="gutterOver(653)"

><td class="source">         if (destIndexType == HardwareIndexBuffer::IT_32BIT)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_654

 onmouseover="gutterOver(654)"

><td class="source">         {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_655

 onmouseover="gutterOver(655)"

><td class="source">            //-- Convert 16 bit to 32 bit indices --<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_656

 onmouseover="gutterOver(656)"

><td class="source">            //Lock the input buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_657

 onmouseover="gutterOver(657)"

><td class="source">            uint16 *source = static_cast&lt;uint16*&gt;(sourceIndexData-&gt;indexBuffer-&gt;lock(<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_658

 onmouseover="gutterOver(658)"

><td class="source">               sourceIndexData-&gt;indexStart, sourceIndexData-&gt;indexCount, HardwareBuffer::HBL_READ_ONLY));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_659

 onmouseover="gutterOver(659)"

><td class="source">            uint16 *sourceEnd = source + sourceIndexData-&gt;indexCount;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_660

 onmouseover="gutterOver(660)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_661

 onmouseover="gutterOver(661)"

><td class="source">            //And copy it to the output buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_662

 onmouseover="gutterOver(662)"

><td class="source">            while (source != sourceEnd)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_663

 onmouseover="gutterOver(663)"

><td class="source">            {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_664

 onmouseover="gutterOver(664)"

><td class="source">               uint32 indx = *source++;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_665

 onmouseover="gutterOver(665)"

><td class="source">               *indexBuffer32++ = (indx + indexOffset);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_666

 onmouseover="gutterOver(666)"

><td class="source">            }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_667

 onmouseover="gutterOver(667)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_668

 onmouseover="gutterOver(668)"

><td class="source">            sourceIndexData-&gt;indexBuffer-&gt;unlock();                  // Unlock the input buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_669

 onmouseover="gutterOver(669)"

><td class="source">            indexOffset += queuedMesh.subMesh-&gt;vertexData-&gt;vertexCount; // Increment the index offset<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_670

 onmouseover="gutterOver(670)"

><td class="source">         }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_671

 onmouseover="gutterOver(671)"

><td class="source">         else<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_672

 onmouseover="gutterOver(672)"

><td class="source">         {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_673

 onmouseover="gutterOver(673)"

><td class="source">            //Lock the input buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_674

 onmouseover="gutterOver(674)"

><td class="source">            uint16 *source = static_cast&lt;uint16*&gt;(sourceIndexData-&gt;indexBuffer-&gt;lock(<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_675

 onmouseover="gutterOver(675)"

><td class="source">               sourceIndexData-&gt;indexStart, sourceIndexData-&gt;indexCount, HardwareBuffer::HBL_READ_ONLY));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_676

 onmouseover="gutterOver(676)"

><td class="source">            uint16 *sourceEnd = source + sourceIndexData-&gt;indexCount;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_677

 onmouseover="gutterOver(677)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_678

 onmouseover="gutterOver(678)"

><td class="source">            //And copy it to the output buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_679

 onmouseover="gutterOver(679)"

><td class="source">            while (source != sourceEnd)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_680

 onmouseover="gutterOver(680)"

><td class="source">               *indexBuffer16++ = static_cast&lt;uint16&gt;(*source++ + indexOffset);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_681

 onmouseover="gutterOver(681)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_682

 onmouseover="gutterOver(682)"

><td class="source">            sourceIndexData-&gt;indexBuffer-&gt;unlock();                  // Unlock the input buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_683

 onmouseover="gutterOver(683)"

><td class="source">            indexOffset += queuedMesh.subMesh-&gt;vertexData-&gt;vertexCount; // Increment the index offset<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_684

 onmouseover="gutterOver(684)"

><td class="source">         }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_685

 onmouseover="gutterOver(685)"

><td class="source">      }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_686

 onmouseover="gutterOver(686)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_687

 onmouseover="gutterOver(687)"

><td class="source">   }  // For each queued mesh<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_688

 onmouseover="gutterOver(688)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_689

 onmouseover="gutterOver(689)"

><td class="source">   //Unlock buffers<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_690

 onmouseover="gutterOver(690)"

><td class="source">   m_pIndexData-&gt;indexBuffer-&gt;unlock();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_691

 onmouseover="gutterOver(691)"

><td class="source">   for (Ogre::ushort i = 0; i &lt; vertBinding-&gt;getBufferCount(); ++i)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_692

 onmouseover="gutterOver(692)"

><td class="source">      vertBinding-&gt;getBuffer(i)-&gt;unlock();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_693

 onmouseover="gutterOver(693)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_694

 onmouseover="gutterOver(694)"

><td class="source">   m_queueMesh.clear();   // Clear mesh queue<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_695

 onmouseover="gutterOver(695)"

><td class="source">   m_Built = true;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_696

 onmouseover="gutterOver(696)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_697

 onmouseover="gutterOver(697)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_698

 onmouseover="gutterOver(698)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_699

 onmouseover="gutterOver(699)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_700

 onmouseover="gutterOver(700)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_701

 onmouseover="gutterOver(701)"

><td class="source">void BatchedGeometry::SubBatch::_buildIdentiryOrientation(const QueuedMesh &amp;queuedMesh,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_702

 onmouseover="gutterOver(702)"

><td class="source">                                                          const Ogre::Vector3 &amp;parentGeomCenter,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_703

 onmouseover="gutterOver(703)"

><td class="source">                                                          const std::vector&lt;VertexDeclaration::VertexElementList&gt; &amp;vertexBufferElements,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_704

 onmouseover="gutterOver(704)"

><td class="source">                                                          std::vector&lt;Ogre::uchar*&gt; &amp;vertexBuffers,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_705

 onmouseover="gutterOver(705)"

><td class="source">                                                          Ogre::VertexData *dstVertexData)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_706

 onmouseover="gutterOver(706)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_707

 onmouseover="gutterOver(707)"

><td class="source">   const VertexData *sourceVertexData = queuedMesh.subMesh-&gt;vertexData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_708

 onmouseover="gutterOver(708)"

><td class="source">   Ogre::Vector3 v3AddBatchPosition = queuedMesh.position - parentGeomCenter;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_709

 onmouseover="gutterOver(709)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_710

 onmouseover="gutterOver(710)"

><td class="source">   //Copy mesh vertex data into the vertex buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_711

 onmouseover="gutterOver(711)"

><td class="source">   VertexBufferBinding *sourceBinds = sourceVertexData-&gt;vertexBufferBinding;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_712

 onmouseover="gutterOver(712)"

><td class="source">   VertexBufferBinding *destBinds = dstVertexData-&gt;vertexBufferBinding;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_713

 onmouseover="gutterOver(713)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_714

 onmouseover="gutterOver(714)"

><td class="source">   // For each vertex buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_715

 onmouseover="gutterOver(715)"

><td class="source">   for (unsigned short ibuffer = 0, bufCnt = destBinds-&gt;getBufferCount(); ibuffer &lt; bufCnt; ++ibuffer)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_716

 onmouseover="gutterOver(716)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_717

 onmouseover="gutterOver(717)"

><td class="source">      if (ibuffer &lt; sourceBinds-&gt;getBufferCount()) // destanation buffer index smaller than source buffers count<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_718

 onmouseover="gutterOver(718)"

><td class="source">      {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_719

 onmouseover="gutterOver(719)"

><td class="source">         //Lock the input buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_720

 onmouseover="gutterOver(720)"

><td class="source">         const HardwareVertexBufferSharedPtr &amp;sourceBuffer = sourceBinds-&gt;getBuffer(ibuffer);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_721

 onmouseover="gutterOver(721)"

><td class="source">         uchar *sourceBase = static_cast&lt;uchar*&gt;(sourceBuffer-&gt;lock(HardwareBuffer::HBL_READ_ONLY));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_722

 onmouseover="gutterOver(722)"

><td class="source">         uchar *destBase = vertexBuffers[ibuffer]; //Get the locked output buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_723

 onmouseover="gutterOver(723)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_724

 onmouseover="gutterOver(724)"

><td class="source">         const VertexDeclaration::VertexElementList &amp;elems = vertexBufferElements[ibuffer];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_725

 onmouseover="gutterOver(725)"

><td class="source">         VertexDeclaration::VertexElementList::const_iterator iBegin = elems.begin(), iEnd = elems.end();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_726

 onmouseover="gutterOver(726)"

><td class="source">         float *sourcePtr = 0, *destPtr = 0;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_727

 onmouseover="gutterOver(727)"

><td class="source">         size_t sourceVertexBuffer = sourceBuffer-&gt;getVertexSize();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_728

 onmouseover="gutterOver(728)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_729

 onmouseover="gutterOver(729)"

><td class="source">         //Copy vertices<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_730

 onmouseover="gutterOver(730)"

><td class="source">         for (size_t v = 0, vertexCounter = sourceVertexData-&gt;vertexCount; v &lt; vertexCounter; ++v)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_731

 onmouseover="gutterOver(731)"

><td class="source">         {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_732

 onmouseover="gutterOver(732)"

><td class="source">            // Iterate over vertex elements<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_733

 onmouseover="gutterOver(733)"

><td class="source">            VertexDeclaration::VertexElementList::const_iterator it = iBegin;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_734

 onmouseover="gutterOver(734)"

><td class="source">            while (it != iEnd)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_735

 onmouseover="gutterOver(735)"

><td class="source">            {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_736

 onmouseover="gutterOver(736)"

><td class="source">               const VertexElement &amp;elem = *it;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_737

 onmouseover="gutterOver(737)"

><td class="source">               elem.baseVertexPointerToElement(sourceBase, &amp;sourcePtr);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_738

 onmouseover="gutterOver(738)"

><td class="source">               elem.baseVertexPointerToElement(destBase, &amp;destPtr);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_739

 onmouseover="gutterOver(739)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_740

 onmouseover="gutterOver(740)"

><td class="source">               switch (elem.getSemantic())<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_741

 onmouseover="gutterOver(741)"

><td class="source">               {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_742

 onmouseover="gutterOver(742)"

><td class="source">               case VES_POSITION:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_743

 onmouseover="gutterOver(743)"

><td class="source">                  {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_744

 onmouseover="gutterOver(744)"

><td class="source">                     Vector3 tmp(sourcePtr[0], sourcePtr[1], sourcePtr[2]);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_745

 onmouseover="gutterOver(745)"

><td class="source">                     tmp *= queuedMesh.scale;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_746

 onmouseover="gutterOver(746)"

><td class="source">                     tmp += v3AddBatchPosition;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_747

 onmouseover="gutterOver(747)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_748

 onmouseover="gutterOver(748)"

><td class="source">                     destPtr[0] = (float)tmp.x; destPtr[1] = (float)tmp.y; destPtr[2] = (float)tmp.z;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_749

 onmouseover="gutterOver(749)"

><td class="source">                  }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_750

 onmouseover="gutterOver(750)"

><td class="source">                  break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_751

 onmouseover="gutterOver(751)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_752

 onmouseover="gutterOver(752)"

><td class="source">               case VES_NORMAL:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_753

 onmouseover="gutterOver(753)"

><td class="source">               case VES_BINORMAL:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_754

 onmouseover="gutterOver(754)"

><td class="source">               case VES_TANGENT:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_755

 onmouseover="gutterOver(755)"

><td class="source">                  {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_756

 onmouseover="gutterOver(756)"

><td class="source">                     // Raw copy<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_757

 onmouseover="gutterOver(757)"

><td class="source">                     destPtr[0] = sourcePtr[0];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_758

 onmouseover="gutterOver(758)"

><td class="source">                     destPtr[1] = sourcePtr[1];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_759

 onmouseover="gutterOver(759)"

><td class="source">                     destPtr[2] = sourcePtr[2];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_760

 onmouseover="gutterOver(760)"

><td class="source">                  }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_761

 onmouseover="gutterOver(761)"

><td class="source">                  break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_762

 onmouseover="gutterOver(762)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_763

 onmouseover="gutterOver(763)"

><td class="source">               case VES_DIFFUSE:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_764

 onmouseover="gutterOver(764)"

><td class="source">                  {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_765

 onmouseover="gutterOver(765)"

><td class="source">                     uint32 tmpColor = *((uint32*)sourcePtr++);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_766

 onmouseover="gutterOver(766)"

><td class="source">                     uint8 tmpR = static_cast&lt;uint8&gt;(((tmpColor) &amp; 0xFF) * queuedMesh.color.r);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_767

 onmouseover="gutterOver(767)"

><td class="source">                     uint8 tmpG = static_cast&lt;uint8&gt;(((tmpColor &gt;&gt; 8) &amp; 0xFF)  * queuedMesh.color.g);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_768

 onmouseover="gutterOver(768)"

><td class="source">                     uint8 tmpB = static_cast&lt;uint8&gt;(((tmpColor &gt;&gt; 16) &amp; 0xFF) * queuedMesh.color.b);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_769

 onmouseover="gutterOver(769)"

><td class="source">                     //uint8 tmpA = 0xFF; //static_cast&lt;uint8&gt;(((tmpColor &gt;&gt; 24) &amp; 0xFF) * queuedMesh.color.a);  // always alpha 1.f<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_770

 onmouseover="gutterOver(770)"

><td class="source">                     //*((uint32*)destPtr) = tmpR | (tmpG &lt;&lt; 8) | (tmpB &lt;&lt; 16) | (tmpA &lt;&lt; 24);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_771

 onmouseover="gutterOver(771)"

><td class="source">                     *((uint32*)destPtr) = tmpR | (tmpG &lt;&lt; 8) | (tmpB &lt;&lt; 16) | (0xFF &lt;&lt; 24);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_772

 onmouseover="gutterOver(772)"

><td class="source">                  }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_773

 onmouseover="gutterOver(773)"

><td class="source">                  break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_774

 onmouseover="gutterOver(774)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_775

 onmouseover="gutterOver(775)"

><td class="source">               default:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_776

 onmouseover="gutterOver(776)"

><td class="source">                  // Raw copy<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_777

 onmouseover="gutterOver(777)"

><td class="source">                  //memcpy(destPtr, sourcePtr, VertexElement::getTypeSize(elem.getType()));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_778

 onmouseover="gutterOver(778)"

><td class="source">                  memcpy(destPtr, sourcePtr, s_vertexType2Size[elem.getType()]);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_779

 onmouseover="gutterOver(779)"

><td class="source">                  break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_780

 onmouseover="gutterOver(780)"

><td class="source">               };<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_781

 onmouseover="gutterOver(781)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_782

 onmouseover="gutterOver(782)"

><td class="source">               ++it;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_783

 onmouseover="gutterOver(783)"

><td class="source">            }  // while Iterate over vertex elements<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_784

 onmouseover="gutterOver(784)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_785

 onmouseover="gutterOver(785)"

><td class="source">            // Increment both pointers<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_786

 onmouseover="gutterOver(786)"

><td class="source">            destBase    += sourceVertexBuffer;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_787

 onmouseover="gutterOver(787)"

><td class="source">            sourceBase  += sourceVertexBuffer;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_788

 onmouseover="gutterOver(788)"

><td class="source">         }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_789

 onmouseover="gutterOver(789)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_790

 onmouseover="gutterOver(790)"

><td class="source">         vertexBuffers[ibuffer] = destBase;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_791

 onmouseover="gutterOver(791)"

><td class="source">         sourceBuffer-&gt;unlock(); // unlock the input buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_792

 onmouseover="gutterOver(792)"

><td class="source">      }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_793

 onmouseover="gutterOver(793)"

><td class="source">      else<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_794

 onmouseover="gutterOver(794)"

><td class="source">      {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_795

 onmouseover="gutterOver(795)"

><td class="source">         size_t bufferNumber = destBinds-&gt;getBufferCount()-1;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_796

 onmouseover="gutterOver(796)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_797

 onmouseover="gutterOver(797)"

><td class="source">         //Get the locked output buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_798

 onmouseover="gutterOver(798)"

><td class="source">         uint32 *startPtr = (uint32*)vertexBuffers[bufferNumber];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_799

 onmouseover="gutterOver(799)"

><td class="source">         uint32 *endPtr = startPtr + sourceVertexData-&gt;vertexCount;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_800

 onmouseover="gutterOver(800)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_801

 onmouseover="gutterOver(801)"

><td class="source">         //Generate color<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_802

 onmouseover="gutterOver(802)"

><td class="source">         uint8 tmpR = static_cast&lt;uint8&gt;(queuedMesh.color.r * 255);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_803

 onmouseover="gutterOver(803)"

><td class="source">         uint8 tmpG = static_cast&lt;uint8&gt;(queuedMesh.color.g * 255);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_804

 onmouseover="gutterOver(804)"

><td class="source">         uint8 tmpB = static_cast&lt;uint8&gt;(queuedMesh.color.b * 255);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_805

 onmouseover="gutterOver(805)"

><td class="source">         uint32 tmpColor = tmpR | (tmpG &lt;&lt; 8) | (tmpB &lt;&lt; 16) | (0xFF &lt;&lt; 24);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_806

 onmouseover="gutterOver(806)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_807

 onmouseover="gutterOver(807)"

><td class="source">         //Copy colors<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_808

 onmouseover="gutterOver(808)"

><td class="source">         while (startPtr &lt; endPtr)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_809

 onmouseover="gutterOver(809)"

><td class="source">            *startPtr++ = tmpColor;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_810

 onmouseover="gutterOver(810)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_811

 onmouseover="gutterOver(811)"

><td class="source">         vertexBuffers[bufferNumber] += sizeof(uint32) * sourceVertexData-&gt;vertexCount;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_812

 onmouseover="gutterOver(812)"

><td class="source">      }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_813

 onmouseover="gutterOver(813)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_814

 onmouseover="gutterOver(814)"

><td class="source">   }  // For each vertex buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_815

 onmouseover="gutterOver(815)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_816

 onmouseover="gutterOver(816)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_817

 onmouseover="gutterOver(817)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_818

 onmouseover="gutterOver(818)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_819

 onmouseover="gutterOver(819)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_820

 onmouseover="gutterOver(820)"

><td class="source">void BatchedGeometry::SubBatch::_buildFullTransform(const QueuedMesh &amp;queuedMesh,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_821

 onmouseover="gutterOver(821)"

><td class="source">                                                    const Ogre::Vector3 &amp;parentGeomCenter,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_822

 onmouseover="gutterOver(822)"

><td class="source">                                                    const std::vector&lt;VertexDeclaration::VertexElementList&gt; &amp;vertexBufferElements,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_823

 onmouseover="gutterOver(823)"

><td class="source">                                                    std::vector&lt;Ogre::uchar*&gt; &amp;vertexBuffers,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_824

 onmouseover="gutterOver(824)"

><td class="source">                                                    Ogre::VertexData *dstVertexData)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_825

 onmouseover="gutterOver(825)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_826

 onmouseover="gutterOver(826)"

><td class="source">   const VertexData *sourceVertexData = queuedMesh.subMesh-&gt;vertexData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_827

 onmouseover="gutterOver(827)"

><td class="source">   // Get rotation matrix for vertex rotation<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_828

 onmouseover="gutterOver(828)"

><td class="source">   Ogre::Matrix3 m3MeshOrientation;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_829

 onmouseover="gutterOver(829)"

><td class="source">   queuedMesh.orientation.ToRotationMatrix(m3MeshOrientation);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_830

 onmouseover="gutterOver(830)"

><td class="source">   const Ogre::Real *mat = m3MeshOrientation[0];   // Ogre::Matrix3 is row major<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_831

 onmouseover="gutterOver(831)"

><td class="source">   Ogre::Vector3 v3AddBatchPosition = queuedMesh.position - parentGeomCenter;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_832

 onmouseover="gutterOver(832)"

><td class="source">   const Ogre::Vector3 &amp;scale = queuedMesh.scale;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_833

 onmouseover="gutterOver(833)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_834

 onmouseover="gutterOver(834)"

><td class="source">   //Copy mesh vertex data into the vertex buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_835

 onmouseover="gutterOver(835)"

><td class="source">   VertexBufferBinding *sourceBinds = sourceVertexData-&gt;vertexBufferBinding;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_836

 onmouseover="gutterOver(836)"

><td class="source">   VertexBufferBinding *destBinds = dstVertexData-&gt;vertexBufferBinding;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_837

 onmouseover="gutterOver(837)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_838

 onmouseover="gutterOver(838)"

><td class="source">   // For each vertex buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_839

 onmouseover="gutterOver(839)"

><td class="source">   for (unsigned short ibuffer = 0, bufCnt = destBinds-&gt;getBufferCount(); ibuffer &lt; bufCnt; ++ibuffer)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_840

 onmouseover="gutterOver(840)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_841

 onmouseover="gutterOver(841)"

><td class="source">      if (ibuffer &lt; sourceBinds-&gt;getBufferCount()) // destanation buffer index smaller than source buffers count<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_842

 onmouseover="gutterOver(842)"

><td class="source">      {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_843

 onmouseover="gutterOver(843)"

><td class="source">         //Lock the input buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_844

 onmouseover="gutterOver(844)"

><td class="source">         const HardwareVertexBufferSharedPtr &amp;sourceBuffer = sourceBinds-&gt;getBuffer(ibuffer);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_845

 onmouseover="gutterOver(845)"

><td class="source">         uchar *sourceBase = static_cast&lt;uchar*&gt;(sourceBuffer-&gt;lock(HardwareBuffer::HBL_READ_ONLY));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_846

 onmouseover="gutterOver(846)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_847

 onmouseover="gutterOver(847)"

><td class="source">         //Get the locked output buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_848

 onmouseover="gutterOver(848)"

><td class="source">         uchar *destBase = vertexBuffers[ibuffer];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_849

 onmouseover="gutterOver(849)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_850

 onmouseover="gutterOver(850)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_851

 onmouseover="gutterOver(851)"

><td class="source">         const VertexDeclaration::VertexElementList &amp;elems = vertexBufferElements[ibuffer];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_852

 onmouseover="gutterOver(852)"

><td class="source">         VertexDeclaration::VertexElementList::const_iterator iBegin = elems.begin(), iEnd = elems.end();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_853

 onmouseover="gutterOver(853)"

><td class="source">         float *sourcePtr = 0, *destPtr = 0;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_854

 onmouseover="gutterOver(854)"

><td class="source">         size_t sourceVertexSize = sourceBuffer-&gt;getVertexSize();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_855

 onmouseover="gutterOver(855)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_856

 onmouseover="gutterOver(856)"

><td class="source">         //Copy vertices<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_857

 onmouseover="gutterOver(857)"

><td class="source">         for (size_t v = 0, vertexCounter = sourceVertexData-&gt;vertexCount; v &lt; vertexCounter; ++v)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_858

 onmouseover="gutterOver(858)"

><td class="source">         {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_859

 onmouseover="gutterOver(859)"

><td class="source">            VertexDeclaration::VertexElementList::const_iterator it = iBegin;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_860

 onmouseover="gutterOver(860)"

><td class="source">            // Iterate over vertex elements<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_861

 onmouseover="gutterOver(861)"

><td class="source">            while (it != iEnd)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_862

 onmouseover="gutterOver(862)"

><td class="source">            {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_863

 onmouseover="gutterOver(863)"

><td class="source">               const VertexElement &amp;elem = *it;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_864

 onmouseover="gutterOver(864)"

><td class="source">               elem.baseVertexPointerToElement(sourceBase, &amp;sourcePtr);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_865

 onmouseover="gutterOver(865)"

><td class="source">               elem.baseVertexPointerToElement(destBase, &amp;destPtr);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_866

 onmouseover="gutterOver(866)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_867

 onmouseover="gutterOver(867)"

><td class="source">               switch (elem.getSemantic())<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_868

 onmouseover="gutterOver(868)"

><td class="source">               {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_869

 onmouseover="gutterOver(869)"

><td class="source">               case VES_POSITION:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_870

 onmouseover="gutterOver(870)"

><td class="source">                  {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_871

 onmouseover="gutterOver(871)"

><td class="source">                     Vector3 tmp(sourcePtr[0] * scale.x, sourcePtr[1] * scale.x, sourcePtr[2] * scale.x);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_872

 onmouseover="gutterOver(872)"

><td class="source">                     // rotate vector by matrix. Ogre::Matrix3::operator* (const Vector3&amp;) is not fast<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_873

 onmouseover="gutterOver(873)"

><td class="source">                     tmp = Ogre::Vector3(<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_874

 onmouseover="gutterOver(874)"

><td class="source">                        mat[0] * tmp.x + mat[1] * tmp.y + mat[2] * tmp.z,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_875

 onmouseover="gutterOver(875)"

><td class="source">                        mat[3] * tmp.x + mat[4] * tmp.y + mat[5] * tmp.z,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_876

 onmouseover="gutterOver(876)"

><td class="source">                        mat[6] * tmp.x + mat[7] * tmp.y + mat[8] * tmp.z);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_877

 onmouseover="gutterOver(877)"

><td class="source">                     tmp += v3AddBatchPosition;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_878

 onmouseover="gutterOver(878)"

><td class="source">                     destPtr[0] = (float)tmp.x;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_879

 onmouseover="gutterOver(879)"

><td class="source">                     destPtr[1] = (float)tmp.y;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_880

 onmouseover="gutterOver(880)"

><td class="source">                     destPtr[2] = (float)tmp.z;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_881

 onmouseover="gutterOver(881)"

><td class="source">                  }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_882

 onmouseover="gutterOver(882)"

><td class="source">                  break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_883

 onmouseover="gutterOver(883)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_884

 onmouseover="gutterOver(884)"

><td class="source">               case VES_NORMAL:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_885

 onmouseover="gutterOver(885)"

><td class="source">               case VES_BINORMAL:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_886

 onmouseover="gutterOver(886)"

><td class="source">               case VES_TANGENT:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_887

 onmouseover="gutterOver(887)"

><td class="source">                  {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_888

 onmouseover="gutterOver(888)"

><td class="source">                     // rotate vector by matrix. Ogre::Matrix3::operator* (const Vector3&amp;) is not fast<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_889

 onmouseover="gutterOver(889)"

><td class="source">                     destPtr[0] = float(mat[0] * sourcePtr[0] + mat[1] * sourcePtr[1] + mat[2] * sourcePtr[2]); // x<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_890

 onmouseover="gutterOver(890)"

><td class="source">                     destPtr[1] = float(mat[3] * sourcePtr[0] + mat[4] * sourcePtr[1] + mat[5] * sourcePtr[2]); // y<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_891

 onmouseover="gutterOver(891)"

><td class="source">                     destPtr[2] = float(mat[6] * sourcePtr[0] + mat[6] * sourcePtr[1] + mat[6] * sourcePtr[2]); // z<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_892

 onmouseover="gutterOver(892)"

><td class="source">                  }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_893

 onmouseover="gutterOver(893)"

><td class="source">                  break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_894

 onmouseover="gutterOver(894)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_895

 onmouseover="gutterOver(895)"

><td class="source">               case VES_DIFFUSE:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_896

 onmouseover="gutterOver(896)"

><td class="source">                  {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_897

 onmouseover="gutterOver(897)"

><td class="source">                     uint32 tmpColor = *((uint32*)sourcePtr);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_898

 onmouseover="gutterOver(898)"

><td class="source">                     uint8 tmpR = static_cast&lt;uint8&gt;(((tmpColor) &amp; 0xFF)       * queuedMesh.color.r);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_899

 onmouseover="gutterOver(899)"

><td class="source">                     uint8 tmpG = static_cast&lt;uint8&gt;(((tmpColor &gt;&gt; 8) &amp; 0xFF)  * queuedMesh.color.g);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_900

 onmouseover="gutterOver(900)"

><td class="source">                     uint8 tmpB = static_cast&lt;uint8&gt;(((tmpColor &gt;&gt; 16) &amp; 0xFF) * queuedMesh.color.b);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_901

 onmouseover="gutterOver(901)"

><td class="source">                     //uint8 tmpA = 0xFF; //static_cast&lt;uint8&gt;(((tmpColor &gt;&gt; 24) &amp; 0xFF) * queuedMesh.color.a);  // always alpha 1.f<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_902

 onmouseover="gutterOver(902)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_903

 onmouseover="gutterOver(903)"

><td class="source">                     //*((uint32*)destPtr) = tmpR | (tmpG &lt;&lt; 8) | (tmpB &lt;&lt; 16) | (tmpA &lt;&lt; 24);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_904

 onmouseover="gutterOver(904)"

><td class="source">                     *((uint32*)destPtr) = tmpR | (tmpG &lt;&lt; 8) | (tmpB &lt;&lt; 16) | (0xFF &lt;&lt; 24);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_905

 onmouseover="gutterOver(905)"

><td class="source">                  }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_906

 onmouseover="gutterOver(906)"

><td class="source">                  break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_907

 onmouseover="gutterOver(907)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_908

 onmouseover="gutterOver(908)"

><td class="source">               default:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_909

 onmouseover="gutterOver(909)"

><td class="source">                  //memcpy(destPtr, sourcePtr, VertexElement::getTypeSize(elem.getType()));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_910

 onmouseover="gutterOver(910)"

><td class="source">                  memcpy(destPtr, sourcePtr, s_vertexType2Size[elem.getType()]);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_911

 onmouseover="gutterOver(911)"

><td class="source">                  break;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_912

 onmouseover="gutterOver(912)"

><td class="source">               };<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_913

 onmouseover="gutterOver(913)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_914

 onmouseover="gutterOver(914)"

><td class="source">               ++it;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_915

 onmouseover="gutterOver(915)"

><td class="source">            }  // for VertexElementList<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_916

 onmouseover="gutterOver(916)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_917

 onmouseover="gutterOver(917)"

><td class="source">            // Increment both pointers<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_918

 onmouseover="gutterOver(918)"

><td class="source">            destBase    += sourceVertexSize;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_919

 onmouseover="gutterOver(919)"

><td class="source">            sourceBase  += sourceVertexSize;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_920

 onmouseover="gutterOver(920)"

><td class="source">         }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_921

 onmouseover="gutterOver(921)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_922

 onmouseover="gutterOver(922)"

><td class="source">         //Unlock the input buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_923

 onmouseover="gutterOver(923)"

><td class="source">         vertexBuffers[ibuffer] = destBase;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_924

 onmouseover="gutterOver(924)"

><td class="source">         sourceBuffer-&gt;unlock();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_925

 onmouseover="gutterOver(925)"

><td class="source">      }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_926

 onmouseover="gutterOver(926)"

><td class="source">      else<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_927

 onmouseover="gutterOver(927)"

><td class="source">      {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_928

 onmouseover="gutterOver(928)"

><td class="source">         size_t bufferNumber = destBinds-&gt;getBufferCount()-1;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_929

 onmouseover="gutterOver(929)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_930

 onmouseover="gutterOver(930)"

><td class="source">         //Get the locked output buffer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_931

 onmouseover="gutterOver(931)"

><td class="source">         uint32 *startPtr = (uint32*)vertexBuffers[bufferNumber];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_932

 onmouseover="gutterOver(932)"

><td class="source">         uint32 *endPtr = startPtr + sourceVertexData-&gt;vertexCount;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_933

 onmouseover="gutterOver(933)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_934

 onmouseover="gutterOver(934)"

><td class="source">         //Generate color<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_935

 onmouseover="gutterOver(935)"

><td class="source">         uint8 tmpR = static_cast&lt;uint8&gt;(queuedMesh.color.r * 255);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_936

 onmouseover="gutterOver(936)"

><td class="source">         uint8 tmpG = static_cast&lt;uint8&gt;(queuedMesh.color.g * 255);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_937

 onmouseover="gutterOver(937)"

><td class="source">         uint8 tmpB = static_cast&lt;uint8&gt;(queuedMesh.color.b * 255);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_938

 onmouseover="gutterOver(938)"

><td class="source">         uint32 tmpColor = tmpR | (tmpG &lt;&lt; 8) | (tmpB &lt;&lt; 16) | (0xFF &lt;&lt; 24);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_939

 onmouseover="gutterOver(939)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_940

 onmouseover="gutterOver(940)"

><td class="source">         //Copy colors<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_941

 onmouseover="gutterOver(941)"

><td class="source">         while (startPtr &lt; endPtr)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_942

 onmouseover="gutterOver(942)"

><td class="source">            *startPtr++ = tmpColor;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_943

 onmouseover="gutterOver(943)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_944

 onmouseover="gutterOver(944)"

><td class="source">         vertexBuffers[bufferNumber] += sizeof(uint32) * sourceVertexData-&gt;vertexCount;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_945

 onmouseover="gutterOver(945)"

><td class="source">      }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_946

 onmouseover="gutterOver(946)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_947

 onmouseover="gutterOver(947)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_948

 onmouseover="gutterOver(948)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_949

 onmouseover="gutterOver(949)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_950

 onmouseover="gutterOver(950)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_951

 onmouseover="gutterOver(951)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_952

 onmouseover="gutterOver(952)"

><td class="source">void BatchedGeometry::SubBatch::clear()<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_953

 onmouseover="gutterOver(953)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_954

 onmouseover="gutterOver(954)"

><td class="source">   //If built, delete the batch<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_955

 onmouseover="gutterOver(955)"

><td class="source">   if (m_Built)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_956

 onmouseover="gutterOver(956)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_957

 onmouseover="gutterOver(957)"

><td class="source">      m_Built = false;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_958

 onmouseover="gutterOver(958)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_959

 onmouseover="gutterOver(959)"

><td class="source">      //Delete buffers<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_960

 onmouseover="gutterOver(960)"

><td class="source">      m_pIndexData-&gt;indexBuffer.setNull();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_961

 onmouseover="gutterOver(961)"

><td class="source">      m_pVertexData-&gt;vertexBufferBinding-&gt;unsetAllBindings();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_962

 onmouseover="gutterOver(962)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_963

 onmouseover="gutterOver(963)"

><td class="source">      //Reset vertex/index count<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_964

 onmouseover="gutterOver(964)"

><td class="source">      m_pVertexData-&gt;vertexStart = 0;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_965

 onmouseover="gutterOver(965)"

><td class="source">      m_pVertexData-&gt;vertexCount = 0;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_966

 onmouseover="gutterOver(966)"

><td class="source">      m_pIndexData-&gt;indexStart = 0;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_967

 onmouseover="gutterOver(967)"

><td class="source">      m_pIndexData-&gt;indexCount = 0;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_968

 onmouseover="gutterOver(968)"

><td class="source">   }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_969

 onmouseover="gutterOver(969)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_970

 onmouseover="gutterOver(970)"

><td class="source">   m_queueMesh.clear(); // Clear mesh queue<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_971

 onmouseover="gutterOver(971)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_972

 onmouseover="gutterOver(972)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_973

 onmouseover="gutterOver(973)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_974

 onmouseover="gutterOver(974)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_975

 onmouseover="gutterOver(975)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_976

 onmouseover="gutterOver(976)"

><td class="source">void BatchedGeometry::SubBatch::addSelfToRenderQueue(RenderQueueGroup *rqg)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_977

 onmouseover="gutterOver(977)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_978

 onmouseover="gutterOver(978)"

><td class="source">   if (!m_Built)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_979

 onmouseover="gutterOver(979)"

><td class="source">      return;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_980

 onmouseover="gutterOver(980)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_981

 onmouseover="gutterOver(981)"

><td class="source">   //Update material technique based on camera distance<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_982

 onmouseover="gutterOver(982)"

><td class="source">   m_pBestTechnique = m_ptrMaterial-&gt;getBestTechnique(m_ptrMaterial-&gt;getLodIndex(<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_983

 onmouseover="gutterOver(983)"

><td class="source">      m_pParentGeom-&gt;m_fMinDistanceSquared * m_pParentGeom-&gt;m_fMinDistanceSquared));<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_984

 onmouseover="gutterOver(984)"

><td class="source">   rqg-&gt;addRenderable(this, m_pBestTechnique, OGRE_RENDERABLE_DEFAULT_PRIORITY);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_985

 onmouseover="gutterOver(985)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_986

 onmouseover="gutterOver(986)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_987

 onmouseover="gutterOver(987)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_988

 onmouseover="gutterOver(988)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_989

 onmouseover="gutterOver(989)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_990

 onmouseover="gutterOver(990)"

><td class="source">void BatchedGeometry::SubBatch::getRenderOperation(RenderOperation&amp; op)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_991

 onmouseover="gutterOver(991)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_992

 onmouseover="gutterOver(992)"

><td class="source">   op.operationType = RenderOperation::OT_TRIANGLE_LIST;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_993

 onmouseover="gutterOver(993)"

><td class="source">   op.srcRenderable = this;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_994

 onmouseover="gutterOver(994)"

><td class="source">   op.useIndexes = true;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_995

 onmouseover="gutterOver(995)"

><td class="source">   op.vertexData = m_pVertexData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_996

 onmouseover="gutterOver(996)"

><td class="source">   op.indexData = m_pIndexData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_997

 onmouseover="gutterOver(997)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_998

 onmouseover="gutterOver(998)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_999

 onmouseover="gutterOver(999)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1000

 onmouseover="gutterOver(1000)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1001

 onmouseover="gutterOver(1001)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1002

 onmouseover="gutterOver(1002)"

><td class="source">Real BatchedGeometry::SubBatch::getSquaredViewDepth(const Camera* cam) const<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1003

 onmouseover="gutterOver(1003)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1004

 onmouseover="gutterOver(1004)"

><td class="source">   Vector3 camVec = m_pParentGeom-&gt;_convertToLocal(cam-&gt;getDerivedPosition()) - m_pParentGeom-&gt;m_vecCenter;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1005

 onmouseover="gutterOver(1005)"

><td class="source">   return camVec.squaredLength();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1006

 onmouseover="gutterOver(1006)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1007

 onmouseover="gutterOver(1007)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1008

 onmouseover="gutterOver(1008)"

><td class="source">//-----------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1009

 onmouseover="gutterOver(1009)"

><td class="source">///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1010

 onmouseover="gutterOver(1010)"

><td class="source">const Ogre::LightList&amp; BatchedGeometry::SubBatch::getLights(void) const<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1011

 onmouseover="gutterOver(1011)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1012

 onmouseover="gutterOver(1012)"

><td class="source">   return m_pParentGeom-&gt;queryLights();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_1013

 onmouseover="gutterOver(1013)"

><td class="source">}<br></td></tr
></table></pre>
<pre><table width="100%"><tr class="cursor_stop cursor_hidden"><td></td></tr></table></pre>
</td>
</tr></table>

 
<script type="text/javascript">
 var lineNumUnderMouse = -1;
 
 function gutterOver(num) {
 gutterOut();
 var newTR = document.getElementById('gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_' + num);
 if (newTR) {
 newTR.className = 'undermouse';
 }
 lineNumUnderMouse = num;
 }
 function gutterOut() {
 if (lineNumUnderMouse != -1) {
 var oldTR = document.getElementById(
 'gr_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_' + lineNumUnderMouse);
 if (oldTR) {
 oldTR.className = '';
 }
 lineNumUnderMouse = -1;
 }
 }
 var numsGenState = {table_base_id: 'nums_table_'};
 var srcGenState = {table_base_id: 'src_table_'};
 var alignerRunning = false;
 var startOver = false;
 function setLineNumberHeights() {
 if (alignerRunning) {
 startOver = true;
 return;
 }
 numsGenState.chunk_id = 0;
 numsGenState.table = document.getElementById('nums_table_0');
 numsGenState.row_num = 0;
 if (!numsGenState.table) {
 return; // Silently exit if no file is present.
 }
 srcGenState.chunk_id = 0;
 srcGenState.table = document.getElementById('src_table_0');
 srcGenState.row_num = 0;
 alignerRunning = true;
 continueToSetLineNumberHeights();
 }
 function rowGenerator(genState) {
 if (genState.row_num < genState.table.rows.length) {
 var currentRow = genState.table.rows[genState.row_num];
 genState.row_num++;
 return currentRow;
 }
 var newTable = document.getElementById(
 genState.table_base_id + (genState.chunk_id + 1));
 if (newTable) {
 genState.chunk_id++;
 genState.row_num = 0;
 genState.table = newTable;
 return genState.table.rows[0];
 }
 return null;
 }
 var MAX_ROWS_PER_PASS = 1000;
 function continueToSetLineNumberHeights() {
 var rowsInThisPass = 0;
 var numRow = 1;
 var srcRow = 1;
 while (numRow && srcRow && rowsInThisPass < MAX_ROWS_PER_PASS) {
 numRow = rowGenerator(numsGenState);
 srcRow = rowGenerator(srcGenState);
 rowsInThisPass++;
 if (numRow && srcRow) {
 if (numRow.offsetHeight != srcRow.offsetHeight) {
 numRow.firstChild.style.height = srcRow.offsetHeight + 'px';
 }
 }
 }
 if (rowsInThisPass >= MAX_ROWS_PER_PASS) {
 setTimeout(continueToSetLineNumberHeights, 10);
 } else {
 alignerRunning = false;
 if (startOver) {
 startOver = false;
 setTimeout(setLineNumberHeights, 500);
 }
 }
 }
 function initLineNumberHeights() {
 // Do 2 complete passes, because there can be races
 // between this code and prettify.
 startOver = true;
 setTimeout(setLineNumberHeights, 250);
 window.onresize = setLineNumberHeights;
 }
 initLineNumberHeights();
</script>

 
 
 <div id="log">
 <div style="text-align:right">
 <a class="ifCollapse" href="#" onclick="_toggleMeta(this); return false">Show details</a>
 <a class="ifExpand" href="#" onclick="_toggleMeta(this); return false">Hide details</a>
 </div>
 <div class="ifExpand">
 
 
 <div class="pmeta_bubble_bg" style="border:1px solid white">
 <div class="round4"></div>
 <div class="round2"></div>
 <div class="round1"></div>
 <div class="box-inner">
 <div id="changelog">
 <p>Change log</p>
 <div>
 <a href="/p/ogre-paged/source/detail?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&amp;r=8bbc960029af74335b963770a933d78e564a859d">8bbc960029af</a>
 by m2codeGEN (SVA)
 on May 25, 2011
 &nbsp; <a href="/p/ogre-paged/source/diff?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=8bbc960029af74335b963770a933d78e564a859d&amp;format=side&amp;path=/source/BatchedGeometry.cpp&amp;old_path=/source/BatchedGeometry.cpp&amp;old=4b9c9c16a1d37aebb2a47614615e6809d17b5ae2">Diff</a>
 </div>
 <pre>Ogre Paged Geometry 1.1.1 RC1</pre>
 </div>
 
 
 
 
 
 
 <script type="text/javascript">
 var detail_url = '/p/ogre-paged/source/detail?r=8bbc960029af74335b963770a933d78e564a859d&spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08';
 var publish_url = '/p/ogre-paged/source/detail?r=8bbc960029af74335b963770a933d78e564a859d&spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08#publish';
 // describe the paths of this revision in javascript.
 var changed_paths = [];
 var changed_urls = [];
 
 changed_paths.push('/include/BatchPage.h');
 changed_urls.push('/p/ogre-paged/source/browse/include/BatchPage.h?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/include/BatchedGeometry.h');
 changed_urls.push('/p/ogre-paged/source/browse/include/BatchedGeometry.h?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/include/GrassLoader.h');
 changed_urls.push('/p/ogre-paged/source/browse/include/GrassLoader.h?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/include/ImpostorPage.h');
 changed_urls.push('/p/ogre-paged/source/browse/include/ImpostorPage.h?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/include/PagedGeometry.h');
 changed_urls.push('/p/ogre-paged/source/browse/include/PagedGeometry.h?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/include/PropertyMaps.h');
 changed_urls.push('/p/ogre-paged/source/browse/include/PropertyMaps.h?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/include/RandomTable.h');
 changed_urls.push('/p/ogre-paged/source/browse/include/RandomTable.h?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/include/TreeLoader2D.h');
 changed_urls.push('/p/ogre-paged/source/browse/include/TreeLoader2D.h?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/include/TreeLoader3D.h');
 changed_urls.push('/p/ogre-paged/source/browse/include/TreeLoader3D.h?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/include/WindBatchPage.h');
 changed_urls.push('/p/ogre-paged/source/browse/include/WindBatchPage.h?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/include/WindBatchedGeometry.h');
 changed_urls.push('/p/ogre-paged/source/browse/include/WindBatchedGeometry.h?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/source/BatchPage.cpp');
 changed_urls.push('/p/ogre-paged/source/browse/source/BatchPage.cpp?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/source/BatchedGeometry.cpp');
 changed_urls.push('/p/ogre-paged/source/browse/source/BatchedGeometry.cpp?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 var selected_path = '/source/BatchedGeometry.cpp';
 
 
 changed_paths.push('/source/GrassLoader.cpp');
 changed_urls.push('/p/ogre-paged/source/browse/source/GrassLoader.cpp?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/source/ImpostorPage.cpp');
 changed_urls.push('/p/ogre-paged/source/browse/source/ImpostorPage.cpp?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/source/PagedGeometry.cpp');
 changed_urls.push('/p/ogre-paged/source/browse/source/PagedGeometry.cpp?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/source/PropertyMaps.cpp');
 changed_urls.push('/p/ogre-paged/source/browse/source/PropertyMaps.cpp?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/source/StaticBillboardSet.cpp');
 changed_urls.push('/p/ogre-paged/source/browse/source/StaticBillboardSet.cpp?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/source/TreeLoader2D.cpp');
 changed_urls.push('/p/ogre-paged/source/browse/source/TreeLoader2D.cpp?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/source/TreeLoader3D.cpp');
 changed_urls.push('/p/ogre-paged/source/browse/source/TreeLoader3D.cpp?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/source/WindBatchPage.cpp');
 changed_urls.push('/p/ogre-paged/source/browse/source/WindBatchPage.cpp?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 changed_paths.push('/source/WindBatchedGeometry.cpp');
 changed_urls.push('/p/ogre-paged/source/browse/source/WindBatchedGeometry.cpp?r\x3d8bbc960029af74335b963770a933d78e564a859d\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 
 function getCurrentPageIndex() {
 for (var i = 0; i < changed_paths.length; i++) {
 if (selected_path == changed_paths[i]) {
 return i;
 }
 }
 }
 function getNextPage() {
 var i = getCurrentPageIndex();
 if (i < changed_paths.length - 1) {
 return changed_urls[i + 1];
 }
 return null;
 }
 function getPreviousPage() {
 var i = getCurrentPageIndex();
 if (i > 0) {
 return changed_urls[i - 1];
 }
 return null;
 }
 function gotoNextPage() {
 var page = getNextPage();
 if (!page) {
 page = detail_url;
 }
 window.location = page;
 }
 function gotoPreviousPage() {
 var page = getPreviousPage();
 if (!page) {
 page = detail_url;
 }
 window.location = page;
 }
 function gotoDetailPage() {
 window.location = detail_url;
 }
 function gotoPublishPage() {
 window.location = publish_url;
 }
</script>

 
 <style type="text/css">
 #review_nav {
 border-top: 3px solid white;
 padding-top: 6px;
 margin-top: 1em;
 }
 #review_nav td {
 vertical-align: middle;
 }
 #review_nav select {
 margin: .5em 0;
 }
 </style>
 <div id="review_nav">
 <table><tr><td>Go to:&nbsp;</td><td>
 <select name="files_in_rev" onchange="window.location=this.value">
 
 <option value="/p/ogre-paged/source/browse/include/BatchPage.h?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/include/BatchPage.h</option>
 
 <option value="/p/ogre-paged/source/browse/include/BatchedGeometry.h?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/include/BatchedGeometry.h</option>
 
 <option value="/p/ogre-paged/source/browse/include/GrassLoader.h?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/include/GrassLoader.h</option>
 
 <option value="/p/ogre-paged/source/browse/include/ImpostorPage.h?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/include/ImpostorPage.h</option>
 
 <option value="/p/ogre-paged/source/browse/include/PagedGeometry.h?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/include/PagedGeometry.h</option>
 
 <option value="/p/ogre-paged/source/browse/include/PropertyMaps.h?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/include/PropertyMaps.h</option>
 
 <option value="/p/ogre-paged/source/browse/include/RandomTable.h?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/include/RandomTable.h</option>
 
 <option value="/p/ogre-paged/source/browse/include/TreeLoader2D.h?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/include/TreeLoader2D.h</option>
 
 <option value="/p/ogre-paged/source/browse/include/TreeLoader3D.h?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/include/TreeLoader3D.h</option>
 
 <option value="/p/ogre-paged/source/browse/include/WindBatchPage.h?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/include/WindBatchPage.h</option>
 
 <option value="/p/ogre-paged/source/browse/include/WindBatchedGeometry.h?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/include/WindBatchedGeometry.h</option>
 
 <option value="/p/ogre-paged/source/browse/source/BatchPage.cpp?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/source/BatchPage.cpp</option>
 
 <option value="/p/ogre-paged/source/browse/source/BatchedGeometry.cpp?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 selected="selected"
 >/source/BatchedGeometry.cpp</option>
 
 <option value="/p/ogre-paged/source/browse/source/GrassLoader.cpp?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/source/GrassLoader.cpp</option>
 
 <option value="/p/ogre-paged/source/browse/source/ImpostorPage.cpp?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/source/ImpostorPage.cpp</option>
 
 <option value="/p/ogre-paged/source/browse/source/PagedGeometry.cpp?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/source/PagedGeometry.cpp</option>
 
 <option value="/p/ogre-paged/source/browse/source/PropertyMaps.cpp?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/source/PropertyMaps.cpp</option>
 
 <option value="/p/ogre-paged/source/browse/source/StaticBillboardSet.cpp?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/source/StaticBillboardSet.cpp</option>
 
 <option value="/p/ogre-paged/source/browse/source/TreeLoader2D.cpp?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/source/TreeLoader2D.cpp</option>
 
 <option value="/p/ogre-paged/source/browse/source/TreeLoader3D.cpp?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/source/TreeLoader3D.cpp</option>
 
 <option value="/p/ogre-paged/source/browse/source/WindBatchPage.cpp?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/source/WindBatchPage.cpp</option>
 
 <option value="/p/ogre-paged/source/browse/source/WindBatchedGeometry.cpp?r=8bbc960029af74335b963770a933d78e564a859d&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 
 >/source/WindBatchedGeometry.cpp</option>
 
 </select>
 </td></tr></table>
 
 
 <div id="review_instr" class="closed">
 <a class="ifOpened" href="/p/ogre-paged/source/detail?r=8bbc960029af74335b963770a933d78e564a859d&spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08#publish">Publish your comments</a>
 <div class="ifClosed">Double click a line to add a comment</div>
 </div>
 
 </div>
 
 
 </div>
 <div class="round1"></div>
 <div class="round2"></div>
 <div class="round4"></div>
 </div>
 <div class="pmeta_bubble_bg" style="border:1px solid white">
 <div class="round4"></div>
 <div class="round2"></div>
 <div class="round1"></div>
 <div class="box-inner">
 <div id="older_bubble">
 <p>Older revisions</p>
 
 
 <div class="closed" style="margin-bottom:3px;" >
 <a class="ifClosed" onclick="return _toggleHidden(this)"><img src="http://www.gstatic.com/codesite/ph/images/plus.gif" ></a>
 <a class="ifOpened" onclick="return _toggleHidden(this)"><img src="http://www.gstatic.com/codesite/ph/images/minus.gif" ></a>
 <a href="/p/ogre-paged/source/detail?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=4b9c9c16a1d37aebb2a47614615e6809d17b5ae2">4b9c9c16a1d3</a>
 by m2codeGEN (SVA)
 on May 16, 2011
 &nbsp; <a href="/p/ogre-paged/source/diff?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=4b9c9c16a1d37aebb2a47614615e6809d17b5ae2&amp;format=side&amp;path=/source/BatchedGeometry.cpp&amp;old_path=/source/BatchedGeometry.cpp&amp;old=7d98729086ee8a6213976ea9c197f506b61a88b1">Diff</a>
 <br>
 <pre class="ifOpened">Perfomance imprivment and some code
refactor</pre>
 </div>
 
 <div class="closed" style="margin-bottom:3px;" >
 <a class="ifClosed" onclick="return _toggleHidden(this)"><img src="http://www.gstatic.com/codesite/ph/images/plus.gif" ></a>
 <a class="ifOpened" onclick="return _toggleHidden(this)"><img src="http://www.gstatic.com/codesite/ph/images/minus.gif" ></a>
 <a href="/p/ogre-paged/source/detail?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=7d98729086ee8a6213976ea9c197f506b61a88b1">7d98729086ee</a>
 by RoRTom
 on Mar 24, 2011
 &nbsp; <a href="/p/ogre-paged/source/diff?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=7d98729086ee8a6213976ea9c197f506b61a88b1&amp;format=side&amp;path=/source/BatchedGeometry.cpp&amp;old_path=/source/BatchedGeometry.cpp&amp;old=39f8aaf4be3c0602d85bd3fda0f99008c2ef1bef">Diff</a>
 <br>
 <pre class="ifOpened">performance improvements</pre>
 </div>
 
 <div class="closed" style="margin-bottom:3px;" >
 <a class="ifClosed" onclick="return _toggleHidden(this)"><img src="http://www.gstatic.com/codesite/ph/images/plus.gif" ></a>
 <a class="ifOpened" onclick="return _toggleHidden(this)"><img src="http://www.gstatic.com/codesite/ph/images/minus.gif" ></a>
 <a href="/p/ogre-paged/source/detail?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=39f8aaf4be3c0602d85bd3fda0f99008c2ef1bef">39f8aaf4be3c</a>
 by rortom
 on Apr 9, 2010
 &nbsp; <a href="/p/ogre-paged/source/diff?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=39f8aaf4be3c0602d85bd3fda0f99008c2ef1bef&amp;format=side&amp;path=/source/BatchedGeometry.cpp&amp;old_path=/source/BatchedGeometry.cpp&amp;old=">Diff</a>
 <br>
 <pre class="ifOpened">added svn files</pre>
 </div>
 
 
 <a href="/p/ogre-paged/source/list?path=/source/BatchedGeometry.cpp&r=8bbc960029af74335b963770a933d78e564a859d">All revisions of this file</a>
 </div>
 </div>
 <div class="round1"></div>
 <div class="round2"></div>
 <div class="round4"></div>
 </div>
 
 <div class="pmeta_bubble_bg" style="border:1px solid white">
 <div class="round4"></div>
 <div class="round2"></div>
 <div class="round1"></div>
 <div class="box-inner">
 <div id="fileinfo_bubble">
 <p>File info</p>
 
 <div>Size: 40943 bytes,
 1013 lines</div>
 
 <div><a href="//ogre-paged.googlecode.com/hg/source/BatchedGeometry.cpp">View raw file</a></div>
 </div>
 
 </div>
 <div class="round1"></div>
 <div class="round2"></div>
 <div class="round4"></div>
 </div>
 </div>
 </div>


</div>

</div>
</div>

<script src="http://www.gstatic.com/codesite/ph/15170358673760952803/js/prettify/prettify.js"></script>
<script type="text/javascript">prettyPrint();</script>


<script src="http://www.gstatic.com/codesite/ph/15170358673760952803/js/source_file_scripts.js"></script>

 <script type="text/javascript" src="http://www.gstatic.com/codesite/ph/15170358673760952803/js/kibbles.js"></script>
 <script type="text/javascript">
 var lastStop = null;
 var initialized = false;
 
 function updateCursor(next, prev) {
 if (prev && prev.element) {
 prev.element.className = 'cursor_stop cursor_hidden';
 }
 if (next && next.element) {
 next.element.className = 'cursor_stop cursor';
 lastStop = next.index;
 }
 }
 
 function pubRevealed(data) {
 updateCursorForCell(data.cellId, 'cursor_stop cursor_hidden');
 if (initialized) {
 reloadCursors();
 }
 }
 
 function draftRevealed(data) {
 updateCursorForCell(data.cellId, 'cursor_stop cursor_hidden');
 if (initialized) {
 reloadCursors();
 }
 }
 
 function draftDestroyed(data) {
 updateCursorForCell(data.cellId, 'nocursor');
 if (initialized) {
 reloadCursors();
 }
 }
 function reloadCursors() {
 kibbles.skipper.reset();
 loadCursors();
 if (lastStop != null) {
 kibbles.skipper.setCurrentStop(lastStop);
 }
 }
 // possibly the simplest way to insert any newly added comments
 // is to update the class of the corresponding cursor row,
 // then refresh the entire list of rows.
 function updateCursorForCell(cellId, className) {
 var cell = document.getElementById(cellId);
 // we have to go two rows back to find the cursor location
 var row = getPreviousElement(cell.parentNode);
 row.className = className;
 }
 // returns the previous element, ignores text nodes.
 function getPreviousElement(e) {
 var element = e.previousSibling;
 if (element.nodeType == 3) {
 element = element.previousSibling;
 }
 if (element && element.tagName) {
 return element;
 }
 }
 function loadCursors() {
 // register our elements with skipper
 var elements = CR_getElements('*', 'cursor_stop');
 var len = elements.length;
 for (var i = 0; i < len; i++) {
 var element = elements[i]; 
 element.className = 'cursor_stop cursor_hidden';
 kibbles.skipper.append(element);
 }
 }
 function toggleComments() {
 CR_toggleCommentDisplay();
 reloadCursors();
 }
 function keysOnLoadHandler() {
 // setup skipper
 kibbles.skipper.addStopListener(
 kibbles.skipper.LISTENER_TYPE.PRE, updateCursor);
 // Set the 'offset' option to return the middle of the client area
 // an option can be a static value, or a callback
 kibbles.skipper.setOption('padding_top', 50);
 // Set the 'offset' option to return the middle of the client area
 // an option can be a static value, or a callback
 kibbles.skipper.setOption('padding_bottom', 100);
 // Register our keys
 kibbles.skipper.addFwdKey("n");
 kibbles.skipper.addRevKey("p");
 kibbles.keys.addKeyPressListener(
 'u', function() { window.location = detail_url; });
 kibbles.keys.addKeyPressListener(
 'r', function() { window.location = detail_url + '#publish'; });
 
 kibbles.keys.addKeyPressListener('j', gotoNextPage);
 kibbles.keys.addKeyPressListener('k', gotoPreviousPage);
 
 
 kibbles.keys.addKeyPressListener('h', toggleComments);
 
 }
 </script>
<script src="http://www.gstatic.com/codesite/ph/15170358673760952803/js/code_review_scripts.js"></script>
<script type="text/javascript">
 function showPublishInstructions() {
 var element = document.getElementById('review_instr');
 if (element) {
 element.className = 'opened';
 }
 }
 var codereviews;
 function revsOnLoadHandler() {
 // register our source container with the commenting code
 var paths = {'svn06de7ef9f7907b57123b0e03476e547a1db6ae08': '/source/BatchedGeometry.cpp'}
 codereviews = CR_controller.setup(
 {"token":"BKkLBik2J6v6PYc0DwT3gu89D9I:1365060230755","assetVersionPath":"http://www.gstatic.com/codesite/ph/15170358673760952803","projectHomeUrl":"/p/ogre-paged","profileUrl":"/u/107319084838887332900/","projectName":"ogre-paged","domainName":null,"loggedInUserEmail":"GeoffTopping@gmail.com","relativeBaseUrl":"","assetHostPath":"http://www.gstatic.com/codesite/ph"}, '', 'svn06de7ef9f7907b57123b0e03476e547a1db6ae08', paths,
 CR_BrowseIntegrationFactory);
 
 // register our source container with the commenting code
 // in this case we're registering the container and the revison
 // associated with the contianer which may be the primary revision
 // or may be a previous revision against which the primary revision
 // of the file is being compared.
 codereviews.registerSourceContainer(document.getElementById('lines'), 'svn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 codereviews.registerActivityListener(CR_ActivityType.REVEAL_DRAFT_PLATE, showPublishInstructions);
 
 codereviews.registerActivityListener(CR_ActivityType.REVEAL_PUB_PLATE, pubRevealed);
 codereviews.registerActivityListener(CR_ActivityType.REVEAL_DRAFT_PLATE, draftRevealed);
 codereviews.registerActivityListener(CR_ActivityType.DISCARD_DRAFT_COMMENT, draftDestroyed);
 
 
 
 
 
 
 
 var initialized = true;
 reloadCursors();
 }
 window.onload = function() {keysOnLoadHandler(); revsOnLoadHandler();};

</script>
<script type="text/javascript" src="http://www.gstatic.com/codesite/ph/15170358673760952803/js/dit_scripts.js"></script>

 
 
 
 <script type="text/javascript" src="http://www.gstatic.com/codesite/ph/15170358673760952803/js/ph_core.js"></script>
 
 
 
 
</div> 

<div id="footer" dir="ltr">
 <div class="text">
 <a href="/projecthosting/terms.html">Terms</a> -
 <a href="http://www.google.com/privacy.html">Privacy</a> -
 <a href="/p/support/">Project Hosting Help</a>
 </div>
</div>
 <div class="hostedBy" style="margin-top: -20px;">
 <span style="vertical-align: top;">Powered by <a href="http://code.google.com/projecthosting/">Google Project Hosting</a></span>
 </div>

 
 


 
 
 <script type="text/javascript">_CS_reportToCsi();</script>
 
 </body>
</html>

