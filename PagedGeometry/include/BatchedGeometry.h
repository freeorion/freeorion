



<!DOCTYPE html>
<html>
<head>
 <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" >
 <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1" >
 
 <meta name="ROBOTS" content="NOARCHIVE">
 
 <link rel="icon" type="image/vnd.microsoft.icon" href="http://www.gstatic.com/codesite/ph/images/phosting.ico">
 
 
 <script type="text/javascript">
 
 
 
 
 var codesite_token = "gY1lHVxICVziY3W34CzbpigjGm0:1365060150409";
 
 
 var CS_env = {"profileUrl":"/u/107319084838887332900/","domainName":null,"loggedInUserEmail":"GeoffTopping@gmail.com","token":"gY1lHVxICVziY3W34CzbpigjGm0:1365060150409","projectHomeUrl":"/p/ogre-paged","assetHostPath":"http://www.gstatic.com/codesite/ph","relativeBaseUrl":"","assetVersionPath":"http://www.gstatic.com/codesite/ph/15170358673760952803","projectName":"ogre-paged"};
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
 
 
 <title>BatchedGeometry.h - 
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
 | <a href="https://www.google.com/accounts/Logout?continue=http%3A%2F%2Fcode.google.com%2Fp%2Fogre-paged%2Fsource%2Fbrowse%2Finclude%2FBatchedGeometry.h" 
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
 <span id="crumb_links" class="ifClosed"><a href="/p/ogre-paged/source/browse/include/">include</a><span class="sp">/&nbsp;</span>BatchedGeometry.h</span>
 
 


 </td>
 
 
 <td nowrap="nowrap" width="33%" align="center">
 <a href="/p/ogre-paged/source/browse/include/BatchedGeometry.h?edit=1"
 ><img src="http://www.gstatic.com/codesite/ph/images/pencil-y14.png"
 class="edit_icon">Edit file</a>
 </td>
 
 
 <td nowrap="nowrap" width="33%" align="right">
 <table cellpadding="0" cellspacing="0" style="font-size: 100%"><tr>
 
 
 <td class="flipper">
 <ul class="leftside">
 
 <li><a href="/p/ogre-paged/source/browse/include/BatchedGeometry.h?r=8bbc960029af74335b963770a933d78e564a859d" title="Previous">&lsaquo;8bbc960029af</a></li>
 
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
></table></pre>
<pre><table width="100%"><tr class="nocursor"><td></td></tr></table></pre>
</td>
<td id="lines">
<pre><table width="100%"><tr class="cursor_stop cursor_hidden"><td></td></tr></table></pre>
<pre class="prettyprint "><table id="src_table_0"><tr
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

><td class="source">#ifndef __BatchedGeometry_H__<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_11

 onmouseover="gutterOver(11)"

><td class="source">#define __BatchedGeometry_H__<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_12

 onmouseover="gutterOver(12)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_13

 onmouseover="gutterOver(13)"

><td class="source">#include &lt;OgrePrerequisites.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_14

 onmouseover="gutterOver(14)"

><td class="source">#include &lt;OgreMovableObject.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_15

 onmouseover="gutterOver(15)"

><td class="source">#include &lt;OgreSceneNode.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_16

 onmouseover="gutterOver(16)"

><td class="source">#include &lt;OgreMaterialManager.h&gt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_17

 onmouseover="gutterOver(17)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_18

 onmouseover="gutterOver(18)"

><td class="source">namespace Forests<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_19

 onmouseover="gutterOver(19)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_20

 onmouseover="gutterOver(20)"

><td class="source">   //--------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_21

 onmouseover="gutterOver(21)"

><td class="source">   /// A &quot;lightweight&quot; version of Ogre::StaticGeometry, which gives you a little<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_22

 onmouseover="gutterOver(22)"

><td class="source">   /// more control over the batch materials, etc.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_23

 onmouseover="gutterOver(23)"

><td class="source">   class BatchedGeometry : public Ogre::MovableObject<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_24

 onmouseover="gutterOver(24)"

><td class="source">   {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_25

 onmouseover="gutterOver(25)"

><td class="source">   public:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_26

 onmouseover="gutterOver(26)"

><td class="source">      //--------------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_27

 onmouseover="gutterOver(27)"

><td class="source">      /// Visible chunk of geometry.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_28

 onmouseover="gutterOver(28)"

><td class="source">      class SubBatch : public Ogre::Renderable<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_29

 onmouseover="gutterOver(29)"

><td class="source">      {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_30

 onmouseover="gutterOver(30)"

><td class="source">      protected:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_31

 onmouseover="gutterOver(31)"

><td class="source">         // A structure defining the desired position/orientation/scale of a batched mesh. The<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_32

 onmouseover="gutterOver(32)"

><td class="source">         // SubMesh is not specified since that can be determined by which MeshQueue this belongs to.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_33

 onmouseover="gutterOver(33)"

><td class="source">         struct QueuedMesh<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_34

 onmouseover="gutterOver(34)"

><td class="source">         {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_35

 onmouseover="gutterOver(35)"

><td class="source">            QueuedMesh(Ogre::SubMesh* sm, const Ogre::Vector3 &amp;pos, const Ogre::Quaternion &amp;ori,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_36

 onmouseover="gutterOver(36)"

><td class="source">               const Ogre::Vector3 &amp;scl, const Ogre::ColourValue &amp;clr, void *userData_ = 0) :<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_37

 onmouseover="gutterOver(37)"

><td class="source">            subMesh     (sm),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_38

 onmouseover="gutterOver(38)"

><td class="source">               position    (pos),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_39

 onmouseover="gutterOver(39)"

><td class="source">               orientation (ori),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_40

 onmouseover="gutterOver(40)"

><td class="source">               scale       (scl),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_41

 onmouseover="gutterOver(41)"

><td class="source">               color       (clr),<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_42

 onmouseover="gutterOver(42)"

><td class="source">               userData    (userData_)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_43

 onmouseover="gutterOver(43)"

><td class="source">            {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_44

 onmouseover="gutterOver(44)"

><td class="source">               // empty<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_45

 onmouseover="gutterOver(45)"

><td class="source">            }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_46

 onmouseover="gutterOver(46)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_47

 onmouseover="gutterOver(47)"

><td class="source">            Ogre::SubMesh*    subMesh;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_48

 onmouseover="gutterOver(48)"

><td class="source">            Ogre::Vector3     position;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_49

 onmouseover="gutterOver(49)"

><td class="source">            Ogre::Quaternion  orientation;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_50

 onmouseover="gutterOver(50)"

><td class="source">            Ogre::Vector3     scale;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_51

 onmouseover="gutterOver(51)"

><td class="source">            Ogre::ColourValue color;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_52

 onmouseover="gutterOver(52)"

><td class="source">            void*             userData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_53

 onmouseover="gutterOver(53)"

><td class="source">         };<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_54

 onmouseover="gutterOver(54)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_55

 onmouseover="gutterOver(55)"

><td class="source">         /// Queue of meshes for build batch of geometry<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_56

 onmouseover="gutterOver(56)"

><td class="source">         typedef std::vector&lt;QueuedMesh&gt; TMeshQueue;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_57

 onmouseover="gutterOver(57)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_58

 onmouseover="gutterOver(58)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_59

 onmouseover="gutterOver(59)"

><td class="source">         // Function section<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_60

 onmouseover="gutterOver(60)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_61

 onmouseover="gutterOver(61)"

><td class="source">      public:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_62

 onmouseover="gutterOver(62)"

><td class="source">         /// Constructor<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_63

 onmouseover="gutterOver(63)"

><td class="source">         SubBatch(BatchedGeometry *parent, Ogre::SubEntity *ent);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_64

 onmouseover="gutterOver(64)"

><td class="source">         /// Destructor<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_65

 onmouseover="gutterOver(65)"

><td class="source">         ~SubBatch();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_66

 onmouseover="gutterOver(66)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_67

 onmouseover="gutterOver(67)"

><td class="source">         /// <br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_68

 onmouseover="gutterOver(68)"

><td class="source">         void addSubEntity(Ogre::SubEntity *ent, const Ogre::Vector3 &amp;position,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_69

 onmouseover="gutterOver(69)"

><td class="source">            const Ogre::Quaternion &amp;orientation, const Ogre::Vector3 &amp;scale,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_70

 onmouseover="gutterOver(70)"

><td class="source">            const Ogre::ColourValue &amp;color = Ogre::ColourValue::White, void* userData = NULL);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_71

 onmouseover="gutterOver(71)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_72

 onmouseover="gutterOver(72)"

><td class="source">         /// Build (assemble a vertex/index buffers) geometry for rendering<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_73

 onmouseover="gutterOver(73)"

><td class="source">         virtual void build();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_74

 onmouseover="gutterOver(74)"

><td class="source">         ///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_75

 onmouseover="gutterOver(75)"

><td class="source">         void clear();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_76

 onmouseover="gutterOver(76)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_77

 onmouseover="gutterOver(77)"

><td class="source">         /// <br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_78

 onmouseover="gutterOver(78)"

><td class="source">         void addSelfToRenderQueue(Ogre::RenderQueueGroup *rqg);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_79

 onmouseover="gutterOver(79)"

><td class="source">         ///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_80

 onmouseover="gutterOver(80)"

><td class="source">         void getRenderOperation(Ogre::RenderOperation&amp; op);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_81

 onmouseover="gutterOver(81)"

><td class="source">         ///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_82

 onmouseover="gutterOver(82)"

><td class="source">         Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_83

 onmouseover="gutterOver(83)"

><td class="source">         ///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_84

 onmouseover="gutterOver(84)"

><td class="source">         const Ogre::LightList&amp; getLights(void) const;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_85

 onmouseover="gutterOver(85)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_86

 onmouseover="gutterOver(86)"

><td class="source">         ///<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_87

 onmouseover="gutterOver(87)"

><td class="source">         void setMaterial(Ogre::MaterialPtr &amp;mat)                 { m_ptrMaterial = mat; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_88

 onmouseover="gutterOver(88)"

><td class="source">         void setMaterialName(const Ogre::String &amp;mat, const Ogre::String &amp;rg =<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_89

 onmouseover="gutterOver(89)"

><td class="source">            Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_90

 onmouseover="gutterOver(90)"

><td class="source">         {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_91

 onmouseover="gutterOver(91)"

><td class="source">            m_ptrMaterial = Ogre::MaterialManager::getSingleton().getByName(mat, rg);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_92

 onmouseover="gutterOver(92)"

><td class="source">         }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_93

 onmouseover="gutterOver(93)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_94

 onmouseover="gutterOver(94)"

><td class="source">         /// Get material name. Be careful, resource group name missing<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_95

 onmouseover="gutterOver(95)"

><td class="source">         const Ogre::String&amp; getMaterialName() const              { return m_ptrMaterial-&gt;getName(); }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_96

 onmouseover="gutterOver(96)"

><td class="source">         Ogre::Technique *getTechnique() const                    { return m_pBestTechnique; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_97

 onmouseover="gutterOver(97)"

><td class="source">         const Ogre::MaterialPtr&amp; getMaterial(void) const         { return m_ptrMaterial; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_98

 onmouseover="gutterOver(98)"

><td class="source">         void getWorldTransforms(Ogre::Matrix4* xform) const      { *xform = m_pParentGeom-&gt;_getParentNodeFullTransform(); }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_99

 onmouseover="gutterOver(99)"

><td class="source">         const Ogre::Quaternion&amp; getWorldOrientation(void) const  { return m_pParentGeom-&gt;m_pSceneNode-&gt;_getDerivedOrientation(); }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_100

 onmouseover="gutterOver(100)"

><td class="source">         const Ogre::Vector3&amp; getWorldPosition(void) const        { return m_pParentGeom-&gt;m_pSceneNode-&gt;_getDerivedPosition(); }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_101

 onmouseover="gutterOver(101)"

><td class="source">         bool castsShadows(void) const                            { return m_pParentGeom-&gt;getCastShadows(); }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_102

 onmouseover="gutterOver(102)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_103

 onmouseover="gutterOver(103)"

><td class="source">         // internal fuctions<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_104

 onmouseover="gutterOver(104)"

><td class="source">      private:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_105

 onmouseover="gutterOver(105)"

><td class="source">         /// Build vertex of QueuedMesh if it have identity orientation<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_106

 onmouseover="gutterOver(106)"

><td class="source">         static void _buildIdentiryOrientation(const QueuedMesh &amp;queuedMesh, const Ogre::Vector3 &amp;parentGeomCenter,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_107

 onmouseover="gutterOver(107)"

><td class="source">            const std::vector&lt;Ogre::VertexDeclaration::VertexElementList&gt; &amp;vertexBufferElements, std::vector&lt;Ogre::uchar*&gt; &amp;vertexBuffers,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_108

 onmouseover="gutterOver(108)"

><td class="source">            Ogre::VertexData *dst);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_109

 onmouseover="gutterOver(109)"

><td class="source">         /// Build vertex of QueuedMesh if it have some orientation<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_110

 onmouseover="gutterOver(110)"

><td class="source">         static void _buildFullTransform(const QueuedMesh &amp;queuedMesh, const Ogre::Vector3 &amp;parentGeomCenter,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_111

 onmouseover="gutterOver(111)"

><td class="source">            const std::vector&lt;Ogre::VertexDeclaration::VertexElementList&gt; &amp;vertexBufferElements, std::vector&lt;Ogre::uchar*&gt; &amp;vertexBuffers,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_112

 onmouseover="gutterOver(112)"

><td class="source">            Ogre::VertexData *dst);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_113

 onmouseover="gutterOver(113)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_114

 onmouseover="gutterOver(114)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_115

 onmouseover="gutterOver(115)"

><td class="source">         // Data section class SubBatch<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_116

 onmouseover="gutterOver(116)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_117

 onmouseover="gutterOver(117)"

><td class="source">      public:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_118

 onmouseover="gutterOver(118)"

><td class="source">         Ogre::VertexData* m_pVertexData;          ///&lt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_119

 onmouseover="gutterOver(119)"

><td class="source">         Ogre::IndexData*  m_pIndexData;           ///&lt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_120

 onmouseover="gutterOver(120)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_121

 onmouseover="gutterOver(121)"

><td class="source">      protected:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_122

 onmouseover="gutterOver(122)"

><td class="source">         bool              m_Built;                ///&lt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_123

 onmouseover="gutterOver(123)"

><td class="source">         bool              m_RequireVertexColors;  ///&lt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_124

 onmouseover="gutterOver(124)"

><td class="source">         Ogre::SubMesh*    m_pSubMesh;             ///&lt; Ogre::SubMesh for Index/Vertex buffers manipulation<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_125

 onmouseover="gutterOver(125)"

><td class="source">         BatchedGeometry*  m_pParentGeom;          ///&lt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_126

 onmouseover="gutterOver(126)"

><td class="source">         Ogre::MaterialPtr m_ptrMaterial;          ///&lt;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_127

 onmouseover="gutterOver(127)"

><td class="source">         TMeshQueue        m_queueMesh;            ///&lt; The list of meshes to be added to this batch<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_128

 onmouseover="gutterOver(128)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_129

 onmouseover="gutterOver(129)"

><td class="source">      private:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_130

 onmouseover="gutterOver(130)"

><td class="source">         Ogre::Technique*  m_pBestTechnique;       ///&lt; Technique recalculated every frame<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_131

 onmouseover="gutterOver(131)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_132

 onmouseover="gutterOver(132)"

><td class="source">      }; // end class SubBatch<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_133

 onmouseover="gutterOver(133)"

><td class="source">      //-----------------------------------------------------------------------<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_134

 onmouseover="gutterOver(134)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_135

 onmouseover="gutterOver(135)"

><td class="source">      /// Stores a list of GeomBatch&#39;es, using a format string (generated with getGeometryFormatString()) as the key value<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_136

 onmouseover="gutterOver(136)"

><td class="source">      typedef std::map&lt;Ogre::String, SubBatch*&gt;    TSubBatchMap;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_137

 onmouseover="gutterOver(137)"

><td class="source">      typedef Ogre::MapIterator&lt;TSubBatchMap&gt;      TSubBatchIterator;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_138

 onmouseover="gutterOver(138)"

><td class="source">      typedef Ogre::ConstMapIterator&lt;TSubBatchMap&gt; TConstSubBatchIterator;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_139

 onmouseover="gutterOver(139)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_140

 onmouseover="gutterOver(140)"

><td class="source">   public:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_141

 onmouseover="gutterOver(141)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_142

 onmouseover="gutterOver(142)"

><td class="source">      /// Constructor<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_143

 onmouseover="gutterOver(143)"

><td class="source">      BatchedGeometry(Ogre::SceneManager *mgr, Ogre::SceneNode *rootSceneNode);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_144

 onmouseover="gutterOver(144)"

><td class="source">      ~BatchedGeometry();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_145

 onmouseover="gutterOver(145)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_146

 onmouseover="gutterOver(146)"

><td class="source">      TConstSubBatchIterator getSubBatchIterator() const    { return TConstSubBatchIterator(m_mapSubBatch); }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_147

 onmouseover="gutterOver(147)"

><td class="source">      TSubBatchIterator getSubBatchIterator()               { return TSubBatchIterator(m_mapSubBatch); }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_148

 onmouseover="gutterOver(148)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_149

 onmouseover="gutterOver(149)"

><td class="source">      virtual void addEntity(Ogre::Entity *ent, const Ogre::Vector3 &amp;position,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_150

 onmouseover="gutterOver(150)"

><td class="source">         const Ogre::Quaternion &amp;orientation = Ogre::Quaternion::IDENTITY,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_151

 onmouseover="gutterOver(151)"

><td class="source">         const Ogre::Vector3 &amp;scale = Ogre::Vector3::UNIT_SCALE,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_152

 onmouseover="gutterOver(152)"

><td class="source">         const Ogre::ColourValue &amp;color = Ogre::ColourValue::White);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_153

 onmouseover="gutterOver(153)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_154

 onmouseover="gutterOver(154)"

><td class="source">      void build();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_155

 onmouseover="gutterOver(155)"

><td class="source">      void clear();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_156

 onmouseover="gutterOver(156)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_157

 onmouseover="gutterOver(157)"

><td class="source">      Ogre::Vector3 _convertToLocal(const Ogre::Vector3 &amp;globalVec) const;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_158

 onmouseover="gutterOver(158)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_159

 onmouseover="gutterOver(159)"

><td class="source">      const Ogre::AxisAlignedBox &amp;getBoundingBox(void) const   { return m_boundsAAB; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_160

 onmouseover="gutterOver(160)"

><td class="source">      Ogre::Real getBoundingRadius(void) const                 { return m_fRadius; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_161

 onmouseover="gutterOver(161)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_162

 onmouseover="gutterOver(162)"

><td class="source">   private:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_163

 onmouseover="gutterOver(163)"

><td class="source">      bool isVisible();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_164

 onmouseover="gutterOver(164)"

><td class="source">      const Ogre::String&amp; getMovableType(void) const;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_165

 onmouseover="gutterOver(165)"

><td class="source">      void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables) { /* empty */ }     <br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_166

 onmouseover="gutterOver(166)"

><td class="source">      void _notifyCurrentCamera(Ogre::Camera *cam);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_167

 onmouseover="gutterOver(167)"

><td class="source">      void _updateRenderQueue(Ogre::RenderQueue *queue);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_168

 onmouseover="gutterOver(168)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_169

 onmouseover="gutterOver(169)"

><td class="source">   protected:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_170

 onmouseover="gutterOver(170)"

><td class="source">      static Ogre::String getFormatString(Ogre::SubEntity *ent);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_171

 onmouseover="gutterOver(171)"

><td class="source">      static void extractVertexDataFromShared(const Ogre::MeshPtr &amp;mesh);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_172

 onmouseover="gutterOver(172)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_173

 onmouseover="gutterOver(173)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_174

 onmouseover="gutterOver(174)"

><td class="source">      // Data section of BatchedGeometry class<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_175

 onmouseover="gutterOver(175)"

><td class="source">   protected:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_176

 onmouseover="gutterOver(176)"

><td class="source">      bool                    m_Built;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_177

 onmouseover="gutterOver(177)"

><td class="source">      bool                    m_BoundsUndefined;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_178

 onmouseover="gutterOver(178)"

><td class="source">      Ogre::Vector3           m_vecCenter;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_179

 onmouseover="gutterOver(179)"

><td class="source">      Ogre::AxisAlignedBox    m_boundsAAB;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_180

 onmouseover="gutterOver(180)"

><td class="source">      TSubBatchMap            m_mapSubBatch;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_181

 onmouseover="gutterOver(181)"

><td class="source">      /// Internal matrix for remap vertex type to vertex size instead call VertexElement::getTypeSize<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_182

 onmouseover="gutterOver(182)"

><td class="source">      static const size_t     s_vertexType2Size[Ogre::VET_COLOUR_ABGR + 1];<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_183

 onmouseover="gutterOver(183)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_184

 onmouseover="gutterOver(184)"

><td class="source">   private:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_185

 onmouseover="gutterOver(185)"

><td class="source">      bool                    m_bWithinFarDistance;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_186

 onmouseover="gutterOver(186)"

><td class="source">      Ogre::Real              m_fRadius;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_187

 onmouseover="gutterOver(187)"

><td class="source">      Ogre::Real              m_fMinDistanceSquared;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_188

 onmouseover="gutterOver(188)"

><td class="source">      Ogre::SceneManager*     m_pSceneMgr;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_189

 onmouseover="gutterOver(189)"

><td class="source">      Ogre::SceneNode*        m_pSceneNode;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_190

 onmouseover="gutterOver(190)"

><td class="source">      Ogre::SceneNode*        m_pParentSceneNode;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_191

 onmouseover="gutterOver(191)"

><td class="source">   };<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_192

 onmouseover="gutterOver(192)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_193

 onmouseover="gutterOver(193)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_194

 onmouseover="gutterOver(194)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_195

 onmouseover="gutterOver(195)"

><td class="source">#endif<br></td></tr
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
 <a href="/p/ogre-paged/source/detail?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&amp;r=b147ea8139abf66dfa1b75844742984d7d462f00">b147ea8139ab</a>
 by tfisc...@nimble
 on May 29, 2011
 &nbsp; <a href="/p/ogre-paged/source/diff?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=b147ea8139abf66dfa1b75844742984d7d462f00&amp;format=side&amp;path=/include/BatchedGeometry.h&amp;old_path=/include/BatchedGeometry.h&amp;old=8bbc960029af74335b963770a933d78e564a859d">Diff</a>
 </div>
 <pre>fixed linux compilation problem. GCC
dislikes variables named &quot;or&quot; as it seems</pre>
 </div>
 
 
 
 
 
 
 <script type="text/javascript">
 var detail_url = '/p/ogre-paged/source/detail?r=b147ea8139abf66dfa1b75844742984d7d462f00&spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08';
 var publish_url = '/p/ogre-paged/source/detail?r=b147ea8139abf66dfa1b75844742984d7d462f00&spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08#publish';
 // describe the paths of this revision in javascript.
 var changed_paths = [];
 var changed_urls = [];
 
 changed_paths.push('/include/BatchedGeometry.h');
 changed_urls.push('/p/ogre-paged/source/browse/include/BatchedGeometry.h?r\x3db147ea8139abf66dfa1b75844742984d7d462f00\x26spec\x3dsvn06de7ef9f7907b57123b0e03476e547a1db6ae08');
 
 var selected_path = '/include/BatchedGeometry.h';
 
 
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
 
 <option value="/p/ogre-paged/source/browse/include/BatchedGeometry.h?r=b147ea8139abf66dfa1b75844742984d7d462f00&amp;spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08"
 selected="selected"
 >/include/BatchedGeometry.h</option>
 
 </select>
 </td></tr></table>
 
 
 <div id="review_instr" class="closed">
 <a class="ifOpened" href="/p/ogre-paged/source/detail?r=b147ea8139abf66dfa1b75844742984d7d462f00&spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08#publish">Publish your comments</a>
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
 <a href="/p/ogre-paged/source/detail?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=8bbc960029af74335b963770a933d78e564a859d">8bbc960029af</a>
 by m2codeGEN (SVA)
 on May 25, 2011
 &nbsp; <a href="/p/ogre-paged/source/diff?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=8bbc960029af74335b963770a933d78e564a859d&amp;format=side&amp;path=/include/BatchedGeometry.h&amp;old_path=/include/BatchedGeometry.h&amp;old=4b9c9c16a1d37aebb2a47614615e6809d17b5ae2">Diff</a>
 <br>
 <pre class="ifOpened">Ogre Paged Geometry 1.1.1 RC1</pre>
 </div>
 
 <div class="closed" style="margin-bottom:3px;" >
 <a class="ifClosed" onclick="return _toggleHidden(this)"><img src="http://www.gstatic.com/codesite/ph/images/plus.gif" ></a>
 <a class="ifOpened" onclick="return _toggleHidden(this)"><img src="http://www.gstatic.com/codesite/ph/images/minus.gif" ></a>
 <a href="/p/ogre-paged/source/detail?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=4b9c9c16a1d37aebb2a47614615e6809d17b5ae2">4b9c9c16a1d3</a>
 by m2codeGEN (SVA)
 on May 16, 2011
 &nbsp; <a href="/p/ogre-paged/source/diff?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=4b9c9c16a1d37aebb2a47614615e6809d17b5ae2&amp;format=side&amp;path=/include/BatchedGeometry.h&amp;old_path=/include/BatchedGeometry.h&amp;old=39f8aaf4be3c0602d85bd3fda0f99008c2ef1bef">Diff</a>
 <br>
 <pre class="ifOpened">Perfomance imprivment and some code
refactor</pre>
 </div>
 
 <div class="closed" style="margin-bottom:3px;" >
 <a class="ifClosed" onclick="return _toggleHidden(this)"><img src="http://www.gstatic.com/codesite/ph/images/plus.gif" ></a>
 <a class="ifOpened" onclick="return _toggleHidden(this)"><img src="http://www.gstatic.com/codesite/ph/images/minus.gif" ></a>
 <a href="/p/ogre-paged/source/detail?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=39f8aaf4be3c0602d85bd3fda0f99008c2ef1bef">39f8aaf4be3c</a>
 by rortom
 on Apr 9, 2010
 &nbsp; <a href="/p/ogre-paged/source/diff?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=39f8aaf4be3c0602d85bd3fda0f99008c2ef1bef&amp;format=side&amp;path=/include/BatchedGeometry.h&amp;old_path=/include/BatchedGeometry.h&amp;old=">Diff</a>
 <br>
 <pre class="ifOpened">added svn files</pre>
 </div>
 
 
 <a href="/p/ogre-paged/source/list?path=/include/BatchedGeometry.h&r=b147ea8139abf66dfa1b75844742984d7d462f00">All revisions of this file</a>
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
 
 <div>Size: 9161 bytes,
 195 lines</div>
 
 <div><a href="//ogre-paged.googlecode.com/hg/include/BatchedGeometry.h">View raw file</a></div>
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
 var paths = {'svn06de7ef9f7907b57123b0e03476e547a1db6ae08': '/include/BatchedGeometry.h'}
 codereviews = CR_controller.setup(
 {"profileUrl":"/u/107319084838887332900/","domainName":null,"loggedInUserEmail":"GeoffTopping@gmail.com","token":"gY1lHVxICVziY3W34CzbpigjGm0:1365060150409","projectHomeUrl":"/p/ogre-paged","assetHostPath":"http://www.gstatic.com/codesite/ph","relativeBaseUrl":"","assetVersionPath":"http://www.gstatic.com/codesite/ph/15170358673760952803","projectName":"ogre-paged"}, '', 'svn06de7ef9f7907b57123b0e03476e547a1db6ae08', paths,
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

 
 


 
 </body>
</html>

