



<!DOCTYPE html>
<html>
<head>
 <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" >
 <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1" >
 
 <meta name="ROBOTS" content="NOARCHIVE">
 
 <link rel="icon" type="image/vnd.microsoft.icon" href="http://www.gstatic.com/codesite/ph/images/phosting.ico">
 
 
 <script type="text/javascript">
 
 
 
 
 var codesite_token = "oiJKzYbWMYHYNnIKb0LF_0MVpho:1365060155036";
 
 
 var CS_env = {"projectName":"ogre-paged","assetVersionPath":"http://www.gstatic.com/codesite/ph/15170358673760952803","projectHomeUrl":"/p/ogre-paged","token":"oiJKzYbWMYHYNnIKb0LF_0MVpho:1365060155036","relativeBaseUrl":"","loggedInUserEmail":"GeoffTopping@gmail.com","profileUrl":"/u/107319084838887332900/","domainName":null,"assetHostPath":"http://www.gstatic.com/codesite/ph"};
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
 
 
 <title>GrassLoader.h - 
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
 | <a href="https://www.google.com/accounts/Logout?continue=http%3A%2F%2Fcode.google.com%2Fp%2Fogre-paged%2Fsource%2Fbrowse%2Finclude%2FGrassLoader.h" 
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
 <span id="crumb_links" class="ifClosed"><a href="/p/ogre-paged/source/browse/include/">include</a><span class="sp">/&nbsp;</span>GrassLoader.h</span>
 
 


 </td>
 
 
 <td nowrap="nowrap" width="33%" align="center">
 <a href="/p/ogre-paged/source/browse/include/GrassLoader.h?edit=1"
 ><img src="http://www.gstatic.com/codesite/ph/images/pencil-y14.png"
 class="edit_icon">Edit file</a>
 </td>
 
 
 <td nowrap="nowrap" width="33%" align="right">
 <table cellpadding="0" cellspacing="0" style="font-size: 100%"><tr>
 
 
 <td class="flipper">
 <ul class="leftside">
 
 <li><a href="/p/ogre-paged/source/browse/include/GrassLoader.h?r=39f8aaf4be3c0602d85bd3fda0f99008c2ef1bef" title="Previous">&lsaquo;39f8aaf4be3c</a></li>
 
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

><td class="source">    1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_7

 onmouseover="gutterOver(7)"

><td class="source">    2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_8

 onmouseover="gutterOver(8)"

><td class="source">    3. This notice may not be removed or altered from any source distribution.<br></td></tr
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

><td class="source">#ifndef __GrassLoader_H__<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_12

 onmouseover="gutterOver(12)"

><td class="source">#define __GrassLoader_H__<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_13

 onmouseover="gutterOver(13)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_14

 onmouseover="gutterOver(14)"

><td class="source">#include &quot;PagedGeometry.h&quot;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_15

 onmouseover="gutterOver(15)"

><td class="source">#include &quot;PropertyMaps.h&quot;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_16

 onmouseover="gutterOver(16)"

><td class="source">#include &quot;RandomTable.h&quot;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_17

 onmouseover="gutterOver(17)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_18

 onmouseover="gutterOver(18)"

><td class="source">#include &quot;OgrePrerequisites.h&quot;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_19

 onmouseover="gutterOver(19)"

><td class="source">#include &quot;OgreMaterial.h&quot;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_20

 onmouseover="gutterOver(20)"

><td class="source">#include &quot;OgrePixelFormat.h&quot;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_21

 onmouseover="gutterOver(21)"

><td class="source">#include &quot;OgreStringConverter.h&quot;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_22

 onmouseover="gutterOver(22)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_23

 onmouseover="gutterOver(23)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_24

 onmouseover="gutterOver(24)"

><td class="source">namespace Forests {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_25

 onmouseover="gutterOver(25)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_26

 onmouseover="gutterOver(26)"

><td class="source">class GrassLayer;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_27

 onmouseover="gutterOver(27)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_28

 onmouseover="gutterOver(28)"

><td class="source">/** \brief A PageLoader-derived object you can use with PagedGeometry to produce realistic grass. <br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_29

 onmouseover="gutterOver(29)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_30

 onmouseover="gutterOver(30)"

><td class="source">Using a GrassLoader is simple - simply create an instance, attach it to your PagedGeometry object<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_31

 onmouseover="gutterOver(31)"

><td class="source">with PagedGeometry::setPageLoader(), and add your grass. Important: For best performance, it is<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_32

 onmouseover="gutterOver(32)"

><td class="source">recommended that you use GrassPage (included in GrassLoader.h) to display geometry loaded by GrassLoader.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_33

 onmouseover="gutterOver(33)"

><td class="source">This page type is designed for best performance with this grass system. BatchPage<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_34

 onmouseover="gutterOver(34)"

><td class="source">will work, although performance will be reduced slightly, and ImpostorPage will run extremely slow.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_35

 onmouseover="gutterOver(35)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_36

 onmouseover="gutterOver(36)"

><td class="source">To add grass, just call addLayer(). addLayer() returns a GrassLayer object pointer, which you should<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_37

 onmouseover="gutterOver(37)"

><td class="source">use to further configure your newly added grass. Properties like size, density, color, animation, etc.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_38

 onmouseover="gutterOver(38)"

><td class="source">can be controlled through the GrassLayer class.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_39

 onmouseover="gutterOver(39)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_40

 onmouseover="gutterOver(40)"

><td class="source">\note By default, the GrassLoader doesn&#39;t know what shape your terrain so all grass will be placed at<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_41

 onmouseover="gutterOver(41)"

><td class="source">0 height. To inform GrassLoader of the shape of your terrain, you must specify a height function<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_42

 onmouseover="gutterOver(42)"

><td class="source">that returns the height (y coordinate) of your terrain at the given x and z coordinates. See<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_43

 onmouseover="gutterOver(43)"

><td class="source">the TreeLoader2D::setHeightFunction() documentation for more information.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_44

 onmouseover="gutterOver(44)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_45

 onmouseover="gutterOver(45)"

><td class="source">\warning If you attempt to use Ogre&#39;s scene queries to get the terrain height,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_46

 onmouseover="gutterOver(46)"

><td class="source">keep in mind that calculating the height of Ogre&#39;s built-in terrain this way can<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_47

 onmouseover="gutterOver(47)"

><td class="source">be VERY slow if not done properly, and may cause stuttering due to long paging delays.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_48

 onmouseover="gutterOver(48)"

><td class="source">*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_49

 onmouseover="gutterOver(49)"

><td class="source">class GrassLoader: public PageLoader<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_50

 onmouseover="gutterOver(50)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_51

 onmouseover="gutterOver(51)"

><td class="source">public:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_52

 onmouseover="gutterOver(52)"

><td class="source">	/** \brief Creates a new GrassLoader object. <br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_53

 onmouseover="gutterOver(53)"

><td class="source">	\param geom The PagedGeometry object that this GrassLoader will be assigned to.*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_54

 onmouseover="gutterOver(54)"

><td class="source">	GrassLoader(PagedGeometry *geom);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_55

 onmouseover="gutterOver(55)"

><td class="source">	~GrassLoader();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_56

 onmouseover="gutterOver(56)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_57

 onmouseover="gutterOver(57)"

><td class="source">	/** \brief Adds a grass layer to the scene.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_58

 onmouseover="gutterOver(58)"

><td class="source">	\param material The initial grass texture to use (this can be changed later).<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_59

 onmouseover="gutterOver(59)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_60

 onmouseover="gutterOver(60)"

><td class="source">	Since all grass is potentially infinite, it is not added like normal entities which<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_61

 onmouseover="gutterOver(61)"

><td class="source">	have a specific position. Instead you add a grass &quot;layer&quot; to the scene. A grass layer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_62

 onmouseover="gutterOver(62)"

><td class="source">	is a &quot;carpet&quot; of a single type of grass that gets applied everywhere in your world.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_63

 onmouseover="gutterOver(63)"

><td class="source">	If you want multiple types of grass with different appearances, you&#39;ll have to add<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_64

 onmouseover="gutterOver(64)"

><td class="source">	a multiple grass layers for each style.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_65

 onmouseover="gutterOver(65)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_66

 onmouseover="gutterOver(66)"

><td class="source">	Of course, a grass layer is not completely uniform. The GrassLayer class contains<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_67

 onmouseover="gutterOver(67)"

><td class="source">	functions to vary grass size and density levels as desired.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_68

 onmouseover="gutterOver(68)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_69

 onmouseover="gutterOver(69)"

><td class="source">	\see GrassLayer class for more information. */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_70

 onmouseover="gutterOver(70)"

><td class="source">	GrassLayer *addLayer(const Ogre::String &amp;material);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_71

 onmouseover="gutterOver(71)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_72

 onmouseover="gutterOver(72)"

><td class="source">	/** \brief Removes and deletes a grass layer from the scene<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_73

 onmouseover="gutterOver(73)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_74

 onmouseover="gutterOver(74)"

><td class="source">	This function simply deletes a GrassLayer previously created with addLayer(). */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_75

 onmouseover="gutterOver(75)"

><td class="source">	void deleteLayer(GrassLayer *layer);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_76

 onmouseover="gutterOver(76)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_77

 onmouseover="gutterOver(77)"

><td class="source">	/** \brief Returns a list of added grass layers.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_78

 onmouseover="gutterOver(78)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_79

 onmouseover="gutterOver(79)"

><td class="source">	This function returns a std::list&lt;GrassLayer*&gt; reference, which contains all grass<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_80

 onmouseover="gutterOver(80)"

><td class="source">	layers which have been added to this GrassLoader. */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_81

 onmouseover="gutterOver(81)"

><td class="source">	inline std::list&lt;GrassLayer*&gt; &amp;getLayerList() { return layerList; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_82

 onmouseover="gutterOver(82)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_83

 onmouseover="gutterOver(83)"

><td class="source">	/** \brief Sets the global wind direction for this GrassLoader.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_84

 onmouseover="gutterOver(84)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_85

 onmouseover="gutterOver(85)"

><td class="source">	GrassLayer animation properties are used to configure the most of the animation<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_86

 onmouseover="gutterOver(86)"

><td class="source">	behavior (sway length, speed, etc.), but wind direction is not included in GrassLayer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_87

 onmouseover="gutterOver(87)"

><td class="source">	since this is really a global property. Using this function, you can set the &quot;global&quot;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_88

 onmouseover="gutterOver(88)"

><td class="source">	wind direction which affects all animated grass associated with this PageLoader.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_89

 onmouseover="gutterOver(89)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_90

 onmouseover="gutterOver(90)"

><td class="source">	Default value is Vector3::UNIT_X.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_91

 onmouseover="gutterOver(91)"

><td class="source">		<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_92

 onmouseover="gutterOver(92)"

><td class="source">	\note This only affects grass layers which have breeze animations enabled.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_93

 onmouseover="gutterOver(93)"

><td class="source">	*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_94

 onmouseover="gutterOver(94)"

><td class="source">	inline void setWindDirection(Ogre::Vector3 &amp;dir) { windDir = dir; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_95

 onmouseover="gutterOver(95)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_96

 onmouseover="gutterOver(96)"

><td class="source">	inline void setBuildEdgesEnabled(bool value) { autoEdgeBuildEnabled=value; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_97

 onmouseover="gutterOver(97)"

><td class="source">	inline bool getBuildEdgesEnabled() { return autoEdgeBuildEnabled; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_98

 onmouseover="gutterOver(98)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_99

 onmouseover="gutterOver(99)"

><td class="source">	/** \brief Returns the global wind direction for this GrassLoader.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_100

 onmouseover="gutterOver(100)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_101

 onmouseover="gutterOver(101)"

><td class="source">	\see setWindDirection() for more information about the wind direction. */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_102

 onmouseover="gutterOver(102)"

><td class="source">	inline Ogre::Vector3 &amp;getWindDirection() { return windDir; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_103

 onmouseover="gutterOver(103)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_104

 onmouseover="gutterOver(104)"

><td class="source">	/** \brief Sets the global density factor for this GrassLoader.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_105

 onmouseover="gutterOver(105)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_106

 onmouseover="gutterOver(106)"

><td class="source">	This function can be used to up-scale or down-scale the density of all grass<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_107

 onmouseover="gutterOver(107)"

><td class="source">	associated with this GrassLoader. This is typically used to provide the user<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_108

 onmouseover="gutterOver(108)"

><td class="source">	the option to reduce grass density for better performance on slower machines.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_109

 onmouseover="gutterOver(109)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_110

 onmouseover="gutterOver(110)"

><td class="source">	Final density values are calculated by multiplying the layer density by this<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_111

 onmouseover="gutterOver(111)"

><td class="source">	density factor. For example, a layer with .4 density and a density factor of .5<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_112

 onmouseover="gutterOver(112)"

><td class="source">	will result in a final density of .2 (.5 * .4)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_113

 onmouseover="gutterOver(113)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_114

 onmouseover="gutterOver(114)"

><td class="source">	By default, the density factor is set to 1.0 so the layer density is not modified.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_115

 onmouseover="gutterOver(115)"

><td class="source">	*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_116

 onmouseover="gutterOver(116)"

><td class="source">	inline void setDensityFactor(float density) { densityFactor = density; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_117

 onmouseover="gutterOver(117)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_118

 onmouseover="gutterOver(118)"

><td class="source">	/** \brief Returns the global density factor for this GrassLoader.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_119

 onmouseover="gutterOver(119)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_120

 onmouseover="gutterOver(120)"

><td class="source">	\see setDensityFactor() for more information about the density factor. */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_121

 onmouseover="gutterOver(121)"

><td class="source">	inline float getDensityFactor() { return densityFactor; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_122

 onmouseover="gutterOver(122)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_123

 onmouseover="gutterOver(123)"

><td class="source">	/** \brief Sets the render queue group the grass will be rendered through<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_124

 onmouseover="gutterOver(124)"

><td class="source">	\param queueID Enumerated value of the queue group to use<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_125

 onmouseover="gutterOver(125)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_126

 onmouseover="gutterOver(126)"

><td class="source">	Like Ogre&#39;s MovableObject::setRenderQueueGroup(), this allows you to customize<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_127

 onmouseover="gutterOver(127)"

><td class="source">	the rendering order of your scene. Since grass is transparent, it&#39;s likely that<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_128

 onmouseover="gutterOver(128)"

><td class="source">	you may encounter alpha-sorting issues between grass and your particle effects,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_129

 onmouseover="gutterOver(129)"

><td class="source">	for example. In this case you can use this function to adjust the rendering order<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_130

 onmouseover="gutterOver(130)"

><td class="source">	of the grass to fix the problem.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_131

 onmouseover="gutterOver(131)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_132

 onmouseover="gutterOver(132)"

><td class="source">	If you don&#39;t call this function, the RENDER_QUEUE_6 queue will be used.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_133

 onmouseover="gutterOver(133)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_134

 onmouseover="gutterOver(134)"

><td class="source">	\note Once grass is loaded and being rendered, this won&#39;t have any effect on it.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_135

 onmouseover="gutterOver(135)"

><td class="source">	Be sure to call this function before the scene begins rendering, otherwise you will<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_136

 onmouseover="gutterOver(136)"

><td class="source">	have to call PagedGeometry::reloadGeometry() to force a reload in order for the changes<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_137

 onmouseover="gutterOver(137)"

><td class="source">	to take effect.	*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_138

 onmouseover="gutterOver(138)"

><td class="source">	inline void setRenderQueueGroup(Ogre::uint8 queueID) { renderQueue = queueID; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_139

 onmouseover="gutterOver(139)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_140

 onmouseover="gutterOver(140)"

><td class="source">	/** \brief Sets the height function used to calculate grass Y coordinates<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_141

 onmouseover="gutterOver(141)"

><td class="source">	\param heightFunction A pointer to a height function<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_142

 onmouseover="gutterOver(142)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_143

 onmouseover="gutterOver(143)"

><td class="source">	Unless you want all your grass placed at 0 height, you need to specify a height function<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_144

 onmouseover="gutterOver(144)"

><td class="source">	so GrassLoader will be able to calculate the Y coordinate. The height function given<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_145

 onmouseover="gutterOver(145)"

><td class="source">	to setHeightFunction() should use the following prototype (although you can name the<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_146

 onmouseover="gutterOver(146)"

><td class="source">	function anything you want):<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_147

 onmouseover="gutterOver(147)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_148

 onmouseover="gutterOver(148)"

><td class="source">	\code<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_149

 onmouseover="gutterOver(149)"

><td class="source">	Real getHeightAt(Real x, Real z, void *userData);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_150

 onmouseover="gutterOver(150)"

><td class="source">	\endcode<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_151

 onmouseover="gutterOver(151)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_152

 onmouseover="gutterOver(152)"

><td class="source">	\note If you&#39;re not using the default coordinate system (where x = right, z = back), the<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_153

 onmouseover="gutterOver(153)"

><td class="source">	x/z parameters will actually be representing the appropriate equivalents.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_154

 onmouseover="gutterOver(154)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_155

 onmouseover="gutterOver(155)"

><td class="source">	The userData parameter allows you to include any additional data you want when your height<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_156

 onmouseover="gutterOver(156)"

><td class="source">	function is called, and is completely optional (although you can&#39;t actually omit it from the<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_157

 onmouseover="gutterOver(157)"

><td class="source">	declaration, you can ignore it). Any userData value you choose to supply to setHeightFunction()<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_158

 onmouseover="gutterOver(158)"

><td class="source">	will be passed on to your height function every time it is called.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_159

 onmouseover="gutterOver(159)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_160

 onmouseover="gutterOver(160)"

><td class="source">	After you&#39;ve defined a height function, using setHeightFunction is easy:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_161

 onmouseover="gutterOver(161)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_162

 onmouseover="gutterOver(162)"

><td class="source">	\code<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_163

 onmouseover="gutterOver(163)"

><td class="source">	pageLoader2D-&gt;setHeightFunction(&amp;getHeightAt);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_164

 onmouseover="gutterOver(164)"

><td class="source">	//Or (if you want to pass additional data on to your height function)...<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_165

 onmouseover="gutterOver(165)"

><td class="source">	pageLoader2D-&gt;setHeightFunction(&amp;getHeightAt, myUserData);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_166

 onmouseover="gutterOver(166)"

><td class="source">	\endcode<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_167

 onmouseover="gutterOver(167)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_168

 onmouseover="gutterOver(168)"

><td class="source">	In most cases, you may not even need to use the extra &quot;userData&quot; parameter, but it&#39;s there in<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_169

 onmouseover="gutterOver(169)"

><td class="source">	the event that your height function needs extra contextual data.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_170

 onmouseover="gutterOver(170)"

><td class="source">	*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_171

 onmouseover="gutterOver(171)"

><td class="source">	void setHeightFunction(Ogre::Real (*heightFunction)(Ogre::Real x, Ogre::Real z, void *userData), void *userData = NULL) {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_172

 onmouseover="gutterOver(172)"

><td class="source">		this-&gt;heightFunction = heightFunction;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_173

 onmouseover="gutterOver(173)"

><td class="source">		heightFunctionUserData = userData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_174

 onmouseover="gutterOver(174)"

><td class="source">	}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_175

 onmouseover="gutterOver(175)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_176

 onmouseover="gutterOver(176)"

><td class="source">	/** INTERNAL FUNCTION - DO NOT USE */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_177

 onmouseover="gutterOver(177)"

><td class="source">	void loadPage(PageInfo &amp;page);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_178

 onmouseover="gutterOver(178)"

><td class="source">	/** INTERNAL FUNCTION - DO NOT USE */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_179

 onmouseover="gutterOver(179)"

><td class="source">	void unloadPage(PageInfo &amp;page);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_180

 onmouseover="gutterOver(180)"

><td class="source">	/** INTERNAL FUNCTION - DO NOT USE */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_181

 onmouseover="gutterOver(181)"

><td class="source">	void frameUpdate();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_182

 onmouseover="gutterOver(182)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_183

 onmouseover="gutterOver(183)"

><td class="source">	static float getRangeRandom(float start, float end);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_184

 onmouseover="gutterOver(184)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_185

 onmouseover="gutterOver(185)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_186

 onmouseover="gutterOver(186)"

><td class="source">private:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_187

 onmouseover="gutterOver(187)"

><td class="source">	friend class GrassLayer;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_188

 onmouseover="gutterOver(188)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_189

 onmouseover="gutterOver(189)"

><td class="source">	//Helper functions<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_190

 onmouseover="gutterOver(190)"

><td class="source">	Ogre::Mesh *generateGrass_QUAD(PageInfo &amp;page, GrassLayer *layer, const float *grassPositions, unsigned int grassCount);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_191

 onmouseover="gutterOver(191)"

><td class="source">	Ogre::Mesh *generateGrass_CROSSQUADS(PageInfo &amp;page, GrassLayer *layer, const float *grassPositions, unsigned int grassCount);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_192

 onmouseover="gutterOver(192)"

><td class="source">	Ogre::Mesh *generateGrass_SPRITE(PageInfo &amp;page, GrassLayer *layer, const float *grassPositions, unsigned int grassCount);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_193

 onmouseover="gutterOver(193)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_194

 onmouseover="gutterOver(194)"

><td class="source">	//List of grass types<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_195

 onmouseover="gutterOver(195)"

><td class="source">	std::list&lt;GrassLayer*&gt; layerList;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_196

 onmouseover="gutterOver(196)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_197

 onmouseover="gutterOver(197)"

><td class="source">	//Height data<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_198

 onmouseover="gutterOver(198)"

><td class="source">	Ogre::Real (*heightFunction)(Ogre::Real x, Ogre::Real z, void *userData);	//Pointer to height function<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_199

 onmouseover="gutterOver(199)"

><td class="source">	void *heightFunctionUserData;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_200

 onmouseover="gutterOver(200)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_201

 onmouseover="gutterOver(201)"

><td class="source">	//Misc.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_202

 onmouseover="gutterOver(202)"

><td class="source">	PagedGeometry *geom;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_203

 onmouseover="gutterOver(203)"

><td class="source">	Ogre::uint8 renderQueue;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_204

 onmouseover="gutterOver(204)"

><td class="source">	float densityFactor;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_205

 onmouseover="gutterOver(205)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_206

 onmouseover="gutterOver(206)"

><td class="source">	// random table<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_207

 onmouseover="gutterOver(207)"

><td class="source">	RandomTable *rTable;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_208

 onmouseover="gutterOver(208)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_209

 onmouseover="gutterOver(209)"

><td class="source">	//Animation<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_210

 onmouseover="gutterOver(210)"

><td class="source">	Ogre::Timer windTimer;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_211

 onmouseover="gutterOver(211)"

><td class="source">	Ogre::Vector3 windDir;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_212

 onmouseover="gutterOver(212)"

><td class="source">	unsigned long lastTime;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_213

 onmouseover="gutterOver(213)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_214

 onmouseover="gutterOver(214)"

><td class="source">	bool autoEdgeBuildEnabled;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_215

 onmouseover="gutterOver(215)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_216

 onmouseover="gutterOver(216)"

><td class="source">	static unsigned long GUID;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_217

 onmouseover="gutterOver(217)"

><td class="source">	static inline Ogre::String getUniqueID()<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_218

 onmouseover="gutterOver(218)"

><td class="source">	{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_219

 onmouseover="gutterOver(219)"

><td class="source">		return &quot;GrassLDR&quot; + Ogre::StringConverter::toString(++GUID);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_220

 onmouseover="gutterOver(220)"

><td class="source">	}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_221

 onmouseover="gutterOver(221)"

><td class="source">};<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_222

 onmouseover="gutterOver(222)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_223

 onmouseover="gutterOver(223)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_224

 onmouseover="gutterOver(224)"

><td class="source">/** \brief A technique used to render grass. Passed to GrassLayer::setRenderTechnique(). */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_225

 onmouseover="gutterOver(225)"

><td class="source">enum GrassTechnique<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_226

 onmouseover="gutterOver(226)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_227

 onmouseover="gutterOver(227)"

><td class="source">	/// Grass constructed of randomly placed and rotated quads<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_228

 onmouseover="gutterOver(228)"

><td class="source">	GRASSTECH_QUAD,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_229

 onmouseover="gutterOver(229)"

><td class="source">	/// Grass constructed of two quads forming a &quot;X&quot; cross shape<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_230

 onmouseover="gutterOver(230)"

><td class="source">	GRASSTECH_CROSSQUADS,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_231

 onmouseover="gutterOver(231)"

><td class="source">	/// Grass constructed of camera-facing billboard quads<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_232

 onmouseover="gutterOver(232)"

><td class="source">	GRASSTECH_SPRITE<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_233

 onmouseover="gutterOver(233)"

><td class="source">};<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_234

 onmouseover="gutterOver(234)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_235

 onmouseover="gutterOver(235)"

><td class="source">/** \brief A technique used to fade grass into the distance. Passed to GrassLayer::setFadeTechnique(). */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_236

 onmouseover="gutterOver(236)"

><td class="source">enum FadeTechnique<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_237

 onmouseover="gutterOver(237)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_238

 onmouseover="gutterOver(238)"

><td class="source">	/// Grass that fades into the distance with transparency. Fairly effective in most cases.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_239

 onmouseover="gutterOver(239)"

><td class="source">	FADETECH_ALPHA,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_240

 onmouseover="gutterOver(240)"

><td class="source">	/// Grass that fades in by &quot;growing&quot; up out of the ground. Very effective when grass fades in against the sky, or with alpha-rejected grass.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_241

 onmouseover="gutterOver(241)"

><td class="source">	FADETECH_GROW,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_242

 onmouseover="gutterOver(242)"

><td class="source">	/// Grass that fades in by slowly becoming opaque while it &quot;grows&quot; up out of the ground. Effective with alpha grass fading in against the sky.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_243

 onmouseover="gutterOver(243)"

><td class="source">	FADETECH_ALPHAGROW<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_244

 onmouseover="gutterOver(244)"

><td class="source">};<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_245

 onmouseover="gutterOver(245)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_246

 onmouseover="gutterOver(246)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_247

 onmouseover="gutterOver(247)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_248

 onmouseover="gutterOver(248)"

><td class="source">/** \brief A data structure giving you full control over grass properties.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_249

 onmouseover="gutterOver(249)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_250

 onmouseover="gutterOver(250)"

><td class="source">Grass is added to the scene through GrassLoader::addLayer(). Through this class you<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_251

 onmouseover="gutterOver(251)"

><td class="source">can configure your grass layer any way you like - size, density, render technique,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_252

 onmouseover="gutterOver(252)"

><td class="source">animation, etc. Simply call the appropriate &quot;set&quot; member function to set the desired property.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_253

 onmouseover="gutterOver(253)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_254

 onmouseover="gutterOver(254)"

><td class="source">Remember that you cannot create or delete layers directly. Layers can only be created<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_255

 onmouseover="gutterOver(255)"

><td class="source">with GrassLoader::addLayer(), and may not be deleted manually (they will be deleted when<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_256

 onmouseover="gutterOver(256)"

><td class="source">the associated GrassLoader is deleted).<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_257

 onmouseover="gutterOver(257)"

><td class="source">*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_258

 onmouseover="gutterOver(258)"

><td class="source">class GrassLayer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_259

 onmouseover="gutterOver(259)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_260

 onmouseover="gutterOver(260)"

><td class="source">public:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_261

 onmouseover="gutterOver(261)"

><td class="source">	/** \brief Sets the material that is applied to all grass billboards/quads */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_262

 onmouseover="gutterOver(262)"

><td class="source">	void setMaterialName(const Ogre::String &amp;matName);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_263

 onmouseover="gutterOver(263)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_264

 onmouseover="gutterOver(264)"

><td class="source">	/** \brief Sets the minimum size that grass quads/billboards will be */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_265

 onmouseover="gutterOver(265)"

><td class="source">	void setMinimumSize(float width, float height);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_266

 onmouseover="gutterOver(266)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_267

 onmouseover="gutterOver(267)"

><td class="source">	/** \brief Sets the maximum size that grass quads/billboards will be */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_268

 onmouseover="gutterOver(268)"

><td class="source">	void setMaximumSize(float width, float height);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_269

 onmouseover="gutterOver(269)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_270

 onmouseover="gutterOver(270)"

><td class="source">	/** \brief Sets the maximum density (measured in grass quads/billboards per square unit) of grass */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_271

 onmouseover="gutterOver(271)"

><td class="source">	void setDensity(float density) { this-&gt;density = density; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_272

 onmouseover="gutterOver(272)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_273

 onmouseover="gutterOver(273)"

><td class="source">	/** \brief Sets a minimum / maximum height where grass may appear<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_274

 onmouseover="gutterOver(274)"

><td class="source">	\param minHeight Sets the minimum height grass may have. 0 = no minimum<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_275

 onmouseover="gutterOver(275)"

><td class="source">	\param maxHeight Sets the maximum height grass may have. 0 = no maximum<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_276

 onmouseover="gutterOver(276)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_277

 onmouseover="gutterOver(277)"

><td class="source">	By default grass appears at all altitudes. You can use this function to restrict grass to a<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_278

 onmouseover="gutterOver(278)"

><td class="source">	certain height range. For example, if sea level is at 100 units Y, you might restrict this<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_279

 onmouseover="gutterOver(279)"

><td class="source">	layer to display only above 100 units (so your grass doesn&#39;t grow under water).<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_280

 onmouseover="gutterOver(280)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_281

 onmouseover="gutterOver(281)"

><td class="source">	It&#39;s possible to use density maps (see setDensityMap()) to achieve similar results, but if<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_282

 onmouseover="gutterOver(282)"

><td class="source">	your density map is extremely low resolution, this function may be the only practical option<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_283

 onmouseover="gutterOver(283)"

><td class="source">	to prevent grass from growing under water (when used in combination with your density map).<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_284

 onmouseover="gutterOver(284)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_285

 onmouseover="gutterOver(285)"

><td class="source">	Setting minHeight to 0 means grass has no minimum height - it can grow as low as necessary.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_286

 onmouseover="gutterOver(286)"

><td class="source">	Similarly, setting maxHeight to 0 means grass has no maximum height - it can grow as high<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_287

 onmouseover="gutterOver(287)"

><td class="source">	as necessary. */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_288

 onmouseover="gutterOver(288)"

><td class="source">	void setHeightRange(float minHeight, float maxHeight = 0) { minY = minHeight; maxY = maxHeight; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_289

 onmouseover="gutterOver(289)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_290

 onmouseover="gutterOver(290)"

><td class="source">	/** \brief Set the maximum slope a grass of blade can be placed on.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_291

 onmouseover="gutterOver(291)"

><td class="source">	\param maxSlopeRatio The maximum slope (h/w ratio) a grass blade is allowed to be placed on.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_292

 onmouseover="gutterOver(292)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_293

 onmouseover="gutterOver(293)"

><td class="source">	This function can be used to set the maximum slope you want your grass to be placed on<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_294

 onmouseover="gutterOver(294)"

><td class="source">	(although it doesn&#39;t work for sprite grass). By default grass is allowed on any slope.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_295

 onmouseover="gutterOver(295)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_296

 onmouseover="gutterOver(296)"

><td class="source">	This version of setMaxSlope() accepts a slope ratio value, where ATan(maxSlopeRatio) =<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_297

 onmouseover="gutterOver(297)"

><td class="source">	maxSlopeAngle. If you wish to provide a maximum slope as an angle, either use the other<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_298

 onmouseover="gutterOver(298)"

><td class="source">	overload of this function, or convert your angle to a slope ratio first with Tan().*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_299

 onmouseover="gutterOver(299)"

><td class="source">	void setMaxSlope(const float maxSlopeRatio) { maxSlope = maxSlopeRatio; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_300

 onmouseover="gutterOver(300)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_301

 onmouseover="gutterOver(301)"

><td class="source">	void setMaxSlope(Ogre::Radian maxSlopeAngle) {<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_302

 onmouseover="gutterOver(302)"

><td class="source">		if (maxSlopeAngle &gt; Ogre::Degree(89.99f))<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_303

 onmouseover="gutterOver(303)"

><td class="source">			maxSlopeAngle = Ogre::Degree(89.99f);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_304

 onmouseover="gutterOver(304)"

><td class="source">		if (maxSlopeAngle &lt; Ogre::Degree(0))<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_305

 onmouseover="gutterOver(305)"

><td class="source">			maxSlopeAngle = Ogre::Degree(0);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_306

 onmouseover="gutterOver(306)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_307

 onmouseover="gutterOver(307)"

><td class="source">		maxSlope = Ogre::Math::Tan(maxSlopeAngle);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_308

 onmouseover="gutterOver(308)"

><td class="source">	}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_309

 onmouseover="gutterOver(309)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_310

 onmouseover="gutterOver(310)"

><td class="source">	/** \brief Get the maximum slope a grass blade can be placed on (as set by setMaxSlope()).<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_311

 onmouseover="gutterOver(311)"

><td class="source">	\returns The currently set maximum slope ratio value (not an angle).<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_312

 onmouseover="gutterOver(312)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_313

 onmouseover="gutterOver(313)"

><td class="source">	This returns the currently set maximum slope which is used to determine what ground is too steep<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_314

 onmouseover="gutterOver(314)"

><td class="source">	for grass to be placed on. Note that this returns the slope as a slope ratio, not an angle. If you<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_315

 onmouseover="gutterOver(315)"

><td class="source">	need an angle value, convert with ATan() (maxSlopeAngle = ATan(maxSlopeRatio)).*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_316

 onmouseover="gutterOver(316)"

><td class="source">	Ogre::Real getMaxSlope() const { return maxSlope; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_317

 onmouseover="gutterOver(317)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_318

 onmouseover="gutterOver(318)"

><td class="source">	/** \brief Sets the density map used for this grass layer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_319

 onmouseover="gutterOver(319)"

><td class="source">	\param mapFile The density map image<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_320

 onmouseover="gutterOver(320)"

><td class="source">	\param channel The color channel(s) to from the image to interpret as density<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_321

 onmouseover="gutterOver(321)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_322

 onmouseover="gutterOver(322)"

><td class="source">	A density map is simply a greyscale image, similar to a heightmap, that specifies the grass<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_323

 onmouseover="gutterOver(323)"

><td class="source">	density on your map. Full pixel intensity indicates that grass should be fully dense at that<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_324

 onmouseover="gutterOver(324)"

><td class="source">	point (the maximum density is specified by GrassLayer::setDensity()), while no pixel intensity<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_325

 onmouseover="gutterOver(325)"

><td class="source">	indicates that no grass should appear at that location.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_326

 onmouseover="gutterOver(326)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_327

 onmouseover="gutterOver(327)"

><td class="source">	The channel parameter allows you to extract the density information from the image&#39;s<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_328

 onmouseover="gutterOver(328)"

><td class="source">	red, green, blue, alpha, or color values. For example, you may store density values in the<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_329

 onmouseover="gutterOver(329)"

><td class="source">	alpha channel, in which case you would use CHANNEL_ALPHA. By default, CHANNEL_COLOR is used,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_330

 onmouseover="gutterOver(330)"

><td class="source">	which means the image color is converted to greyscale internally and used as a density map.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_331

 onmouseover="gutterOver(331)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_332

 onmouseover="gutterOver(332)"

><td class="source">	Note that GrassLayer by default has no idea of your terrain/world boundaries, so you<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_333

 onmouseover="gutterOver(333)"

><td class="source">	must specify a rectangular/square area of your world that is affected by density/color maps.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_334

 onmouseover="gutterOver(334)"

><td class="source">	To do this, use the setMapBounds() function. Normally this is set to your terrain&#39;s bounds<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_335

 onmouseover="gutterOver(335)"

><td class="source">	so the density/color map is aligned to your heightmap, but you could apply it anywhere you<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_336

 onmouseover="gutterOver(336)"

><td class="source">	want. */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_337

 onmouseover="gutterOver(337)"

><td class="source">	void setDensityMap(const Ogre::String &amp;mapFile, MapChannel channel = CHANNEL_COLOR);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_338

 onmouseover="gutterOver(338)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_339

 onmouseover="gutterOver(339)"

><td class="source">	/** \brief Sets the density map used for this grass layer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_340

 onmouseover="gutterOver(340)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_341

 onmouseover="gutterOver(341)"

><td class="source">	Overloaded to accept a Texture object. See the original setDensityMap() documentation above<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_342

 onmouseover="gutterOver(342)"

><td class="source">	for more detailed information on density maps.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_343

 onmouseover="gutterOver(343)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_344

 onmouseover="gutterOver(344)"

><td class="source">	\note The texture data you provide is copied into the GrassLayer&#39;s own memory space, so you<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_345

 onmouseover="gutterOver(345)"

><td class="source">	can delete the texture after calling this function without risk of crashing. */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_346

 onmouseover="gutterOver(346)"

><td class="source">	void setDensityMap(Ogre::TexturePtr map, MapChannel channel = CHANNEL_COLOR);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_347

 onmouseover="gutterOver(347)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_348

 onmouseover="gutterOver(348)"

><td class="source">	/** \brief Sets the filtering mode used for density maps<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_349

 onmouseover="gutterOver(349)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_350

 onmouseover="gutterOver(350)"

><td class="source">	This function can be used to set the filtering mode used for your density map when generating<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_351

 onmouseover="gutterOver(351)"

><td class="source">	grass. By default, bilinear filtering is used (MAPFILTER_BILINEAR). If you disable filtering<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_352

 onmouseover="gutterOver(352)"

><td class="source">	by using MAPFILTER_NONE, the resulting layout of your grass may look square and blocky,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_353

 onmouseover="gutterOver(353)"

><td class="source">	depending on the resolution of your density map.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_354

 onmouseover="gutterOver(354)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_355

 onmouseover="gutterOver(355)"

><td class="source">	MAPFILTER_NONE is slightly faster than MAPFILTER_BILINEAR, so use it if you don&#39;t notice any<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_356

 onmouseover="gutterOver(356)"

><td class="source">	considerable blockiness.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_357

 onmouseover="gutterOver(357)"

><td class="source">	*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_358

 onmouseover="gutterOver(358)"

><td class="source">	void setDensityMapFilter(MapFilter filter);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_359

 onmouseover="gutterOver(359)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_360

 onmouseover="gutterOver(360)"

><td class="source">	/** \brief Sets the color map used for this grass layer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_361

 onmouseover="gutterOver(361)"

><td class="source">	\param mapFile The color map image<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_362

 onmouseover="gutterOver(362)"

><td class="source">	\param channel The color channel(s) to from the image to use<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_363

 onmouseover="gutterOver(363)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_364

 onmouseover="gutterOver(364)"

><td class="source">	A color map is simply a texture that allows you to vary the color and shading of grass<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_365

 onmouseover="gutterOver(365)"

><td class="source">	across your world for a more realistic look. For example, adding a dark spot to the center<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_366

 onmouseover="gutterOver(366)"

><td class="source">	of your color map will make grass near the center of your terrain look darker, as long as<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_367

 onmouseover="gutterOver(367)"

><td class="source">	you have the color map aligned to your terrain (see setMapBounds()).<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_368

 onmouseover="gutterOver(368)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_369

 onmouseover="gutterOver(369)"

><td class="source">	The channel parameter allows you to extract the color information from the image&#39;s<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_370

 onmouseover="gutterOver(370)"

><td class="source">	red, green, blue, alpha, or color values. For example, you may store the desired shade of your<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_371

 onmouseover="gutterOver(371)"

><td class="source">	grass in the red channel of an image, in which case you would use CHANNEL_RED (when you choose<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_372

 onmouseover="gutterOver(372)"

><td class="source">	a single channel, it is converted to a greyscale color). By default, CHANNEL_COLOR is used,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_373

 onmouseover="gutterOver(373)"

><td class="source">	which uses the full color information available in the image.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_374

 onmouseover="gutterOver(374)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_375

 onmouseover="gutterOver(375)"

><td class="source">	Remember that GrassLayer by default has no idea of your terrain/world boundaries, so you<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_376

 onmouseover="gutterOver(376)"

><td class="source">	must specify a rectangular/square area of your world that is affected by density/color maps.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_377

 onmouseover="gutterOver(377)"

><td class="source">	To do this, use the setMapBounds() function. Normally this is set to your terrain&#39;s bounds<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_378

 onmouseover="gutterOver(378)"

><td class="source">	so the density/color map is aligned to your heightmap, but you could apply it anywhere you<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_379

 onmouseover="gutterOver(379)"

><td class="source">	want. */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_380

 onmouseover="gutterOver(380)"

><td class="source">	void setColorMap(const Ogre::String &amp;mapFile, MapChannel channel = CHANNEL_COLOR);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_381

 onmouseover="gutterOver(381)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_382

 onmouseover="gutterOver(382)"

><td class="source">	/** \brief Sets the color map used for this grass layer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_383

 onmouseover="gutterOver(383)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_384

 onmouseover="gutterOver(384)"

><td class="source">	Overloaded to accept a Texture object. See the original setColorMap() documentation above<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_385

 onmouseover="gutterOver(385)"

><td class="source">	for more detailed information on color maps.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_386

 onmouseover="gutterOver(386)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_387

 onmouseover="gutterOver(387)"

><td class="source">	\note The texture data you provide is copied into RAM, so you can delete the texture after<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_388

 onmouseover="gutterOver(388)"

><td class="source">	calling this function without risk of crashing. */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_389

 onmouseover="gutterOver(389)"

><td class="source">	void setColorMap(Ogre::TexturePtr map, MapChannel channel = CHANNEL_COLOR);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_390

 onmouseover="gutterOver(390)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_391

 onmouseover="gutterOver(391)"

><td class="source">	/** \brief Sets the filtering mode used for color maps<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_392

 onmouseover="gutterOver(392)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_393

 onmouseover="gutterOver(393)"

><td class="source">	This function can be used to set the filtering mode used for your color map when generating<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_394

 onmouseover="gutterOver(394)"

><td class="source">	grass. By default, bilinear filtering is used (MAPFILTER_BILINEAR). If you disable filtering<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_395

 onmouseover="gutterOver(395)"

><td class="source">	by using MAPFILTER_NONE, the resulting grass coloration may appear slightly pixelated,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_396

 onmouseover="gutterOver(396)"

><td class="source">	depending on the resolution of your color map.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_397

 onmouseover="gutterOver(397)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_398

 onmouseover="gutterOver(398)"

><td class="source">	MAPFILTER_NONE is slightly faster than MAPFILTER_BILINEAR, so use it if you don&#39;t notice any<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_399

 onmouseover="gutterOver(399)"

><td class="source">	considerable pixelation.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_400

 onmouseover="gutterOver(400)"

><td class="source">	*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_401

 onmouseover="gutterOver(401)"

><td class="source">	void setColorMapFilter(MapFilter filter);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_402

 onmouseover="gutterOver(402)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_403

 onmouseover="gutterOver(403)"

><td class="source">	/** \brief Sets the boundaries of the density/color maps<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_404

 onmouseover="gutterOver(404)"

><td class="source">	\param bounds The map boundary<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_405

 onmouseover="gutterOver(405)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_406

 onmouseover="gutterOver(406)"

><td class="source">	By default, the GrassLayer has no knowledge of your terrain/world boundaries, so you must<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_407

 onmouseover="gutterOver(407)"

><td class="source">	use this function to specify a rectangular/square area of your world, otherwise density/color maps<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_408

 onmouseover="gutterOver(408)"

><td class="source">	won&#39;t work properly. The boundary given to this function defines the area where density/color<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_409

 onmouseover="gutterOver(409)"

><td class="source">	maps take effect. Normally this is set to your terrain&#39;s bounds so the density/color map is aligned<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_410

 onmouseover="gutterOver(410)"

><td class="source">	to your heightmap, but you could apply it anywhere you want.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_411

 onmouseover="gutterOver(411)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_412

 onmouseover="gutterOver(412)"

><td class="source">	\note The grass system is infinite, so there&#39;s no need to worry about using too expansive<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_413

 onmouseover="gutterOver(413)"

><td class="source">	boundaries. This setting simply configures the behavior of density and color maps. */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_414

 onmouseover="gutterOver(414)"

><td class="source">	void setMapBounds(const TBounds &amp;bounds)<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_415

 onmouseover="gutterOver(415)"

><td class="source">	{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_416

 onmouseover="gutterOver(416)"

><td class="source">		mapBounds = bounds;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_417

 onmouseover="gutterOver(417)"

><td class="source">	}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_418

 onmouseover="gutterOver(418)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_419

 onmouseover="gutterOver(419)"

><td class="source">	/** \brief Gets a pointer to the density map being used<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_420

 onmouseover="gutterOver(420)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_421

 onmouseover="gutterOver(421)"

><td class="source">	You can use this function to access the internal density map object used by the GrassLoader.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_422

 onmouseover="gutterOver(422)"

><td class="source">	Through this object you can directly manipulate the pixels of the density map, among other<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_423

 onmouseover="gutterOver(423)"

><td class="source">	things.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_424

 onmouseover="gutterOver(424)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_425

 onmouseover="gutterOver(425)"

><td class="source">	Note that although you can edit the density map in real-time through this class, the changes<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_426

 onmouseover="gutterOver(426)"

><td class="source">	won&#39;t be uploaded to your video card until you call PagedGeometry::reloadGeometry(). If you<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_427

 onmouseover="gutterOver(427)"

><td class="source">	don&#39;t, the grass you see will remain unchanged. */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_428

 onmouseover="gutterOver(428)"

><td class="source">	DensityMap* getDensityMap() const { return densityMap; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_429

 onmouseover="gutterOver(429)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_430

 onmouseover="gutterOver(430)"

><td class="source">	/** \brief Gets a pointer to the color map being used<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_431

 onmouseover="gutterOver(431)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_432

 onmouseover="gutterOver(432)"

><td class="source">	You can use this function to access the internal color map object used by the GrassLoader.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_433

 onmouseover="gutterOver(433)"

><td class="source">	Through this object you can directly manipulate the pixels of the color map, among other<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_434

 onmouseover="gutterOver(434)"

><td class="source">	things.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_435

 onmouseover="gutterOver(435)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_436

 onmouseover="gutterOver(436)"

><td class="source">	Note that although you can edit the color map in real-time through this class, the changes<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_437

 onmouseover="gutterOver(437)"

><td class="source">	won&#39;t be uploaded to your video card until you call PagedGeometry::reloadGeometry(). If you<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_438

 onmouseover="gutterOver(438)"

><td class="source">	don&#39;t, the grass you see will remain unchanged. */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_439

 onmouseover="gutterOver(439)"

><td class="source">	ColorMap* getColorMap() const { return colorMap; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_440

 onmouseover="gutterOver(440)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_441

 onmouseover="gutterOver(441)"

><td class="source">	/** \brief Sets the technique used to render this grass layer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_442

 onmouseover="gutterOver(442)"

><td class="source">	\param style The GrassTechnique style used to display grass.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_443

 onmouseover="gutterOver(443)"

><td class="source">	\param blendBase Whether or not grass base blending is enabled.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_444

 onmouseover="gutterOver(444)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_445

 onmouseover="gutterOver(445)"

><td class="source">	The &quot;style&quot; setting allows you to choose from various construction methods, such as<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_446

 onmouseover="gutterOver(446)"

><td class="source">	sprite-style grass quads, plain 3D quads, etc. See the GrassTechnique documentation<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_447

 onmouseover="gutterOver(447)"

><td class="source">	for more information about this option. GRASSTECH_QUAD is used by default.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_448

 onmouseover="gutterOver(448)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_449

 onmouseover="gutterOver(449)"

><td class="source">	Setting &quot;blendBase&quot; to true will enable grass base blending, a technique which helps<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_450

 onmouseover="gutterOver(450)"

><td class="source">	reduce the unnatural flat appearance of grass quads near the camera. Since the flatness<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_451

 onmouseover="gutterOver(451)"

><td class="source">	is most obvious where the grass intersects the terrain, this technique attempts to<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_452

 onmouseover="gutterOver(452)"

><td class="source">	smoothly blend the base of near-by grass into the terrain.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_453

 onmouseover="gutterOver(453)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_454

 onmouseover="gutterOver(454)"

><td class="source">	\note Base blending does not work well with alpha-rejected textures.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_455

 onmouseover="gutterOver(455)"

><td class="source">	*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_456

 onmouseover="gutterOver(456)"

><td class="source">	void setRenderTechnique(GrassTechnique style, bool blendBase = false);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_457

 onmouseover="gutterOver(457)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_458

 onmouseover="gutterOver(458)"

><td class="source">	/** \brief Sets the technique used when fading out distant grass<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_459

 onmouseover="gutterOver(459)"

><td class="source">	\param style The FadeTechnique style used to fade grass.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_460

 onmouseover="gutterOver(460)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_461

 onmouseover="gutterOver(461)"

><td class="source">	This &quot;style&quot; setting allows you to choose from various fade techniques. Depending on<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_462

 onmouseover="gutterOver(462)"

><td class="source">	your scene, certain techniques may look better than others. The most compatible method<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_463

 onmouseover="gutterOver(463)"

><td class="source">	is FADETECH_ALPHA (used by default), although better results can usually be achieved<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_464

 onmouseover="gutterOver(464)"

><td class="source">	with other methods. See the FadeTechnique documentation for more information.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_465

 onmouseover="gutterOver(465)"

><td class="source">	*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_466

 onmouseover="gutterOver(466)"

><td class="source">	void setFadeTechnique(FadeTechnique style);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_467

 onmouseover="gutterOver(467)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_468

 onmouseover="gutterOver(468)"

><td class="source">	/** \brief Enables/disables animation on this layer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_469

 onmouseover="gutterOver(469)"

><td class="source">	<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_470

 onmouseover="gutterOver(470)"

><td class="source">	Always use this function to disable animation, rather than setting SwayLength or SwaySpeed<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_471

 onmouseover="gutterOver(471)"

><td class="source">	to 0. This function will use a different vertex shader which means improved performance<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_472

 onmouseover="gutterOver(472)"

><td class="source">	when animation is disabled.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_473

 onmouseover="gutterOver(473)"

><td class="source">	*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_474

 onmouseover="gutterOver(474)"

><td class="source">	void setAnimationEnabled(bool enabled);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_475

 onmouseover="gutterOver(475)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_476

 onmouseover="gutterOver(476)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_477

 onmouseover="gutterOver(477)"

><td class="source">	/** \brief Enables/disables lighting on this layer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_478

 onmouseover="gutterOver(478)"

><td class="source">	*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_479

 onmouseover="gutterOver(479)"

><td class="source">	void setLightingEnabled(bool enabled);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_480

 onmouseover="gutterOver(480)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_481

 onmouseover="gutterOver(481)"

><td class="source">	/** \brief Sets how far grass should sway back and forth<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_482

 onmouseover="gutterOver(482)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_483

 onmouseover="gutterOver(483)"

><td class="source">	\note Since this is measured in world units, you may have to adjust this depending on<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_484

 onmouseover="gutterOver(484)"

><td class="source">	the size of your grass as set by setMinimumSize() and setMaximumSize().*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_485

 onmouseover="gutterOver(485)"

><td class="source">	void setSwayLength(Ogre::Real mag) { animMag = mag; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_486

 onmouseover="gutterOver(486)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_487

 onmouseover="gutterOver(487)"

><td class="source">	/** \brief Sets the sway speed of the grass (measured in &quot;sways-per-second&quot;) */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_488

 onmouseover="gutterOver(488)"

><td class="source">	void setSwaySpeed(Ogre::Real speed) { animSpeed = speed; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_489

 onmouseover="gutterOver(489)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_490

 onmouseover="gutterOver(490)"

><td class="source">	/** \brief Sets the smooth distribution (positional phase shift) of the grass swaying animation<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_491

 onmouseover="gutterOver(491)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_492

 onmouseover="gutterOver(492)"

><td class="source">	If you set this to 0, grass animation will look very unnatural, since all the grass sway motions<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_493

 onmouseover="gutterOver(493)"

><td class="source">	will be in perfect synchronization (everything sways to the right, then everything sways to the<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_494

 onmouseover="gutterOver(494)"

><td class="source">	left, etc.) This sets the &quot;positional phase shift&quot;, which gives the grass a &quot;wave&quot; like phase<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_495

 onmouseover="gutterOver(495)"

><td class="source">	distribution. The higher this value is, the more &quot;chaotic&quot; the wind will appear. Lower values give<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_496

 onmouseover="gutterOver(496)"

><td class="source">	a smoother breeze appearance, but values too high can look unrealistic.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_497

 onmouseover="gutterOver(497)"

><td class="source">	*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_498

 onmouseover="gutterOver(498)"

><td class="source">	void setSwayDistribution(Ogre::Real freq) { animFreq = freq; }<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_499

 onmouseover="gutterOver(499)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_500

 onmouseover="gutterOver(500)"

><td class="source">private:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_501

 onmouseover="gutterOver(501)"

><td class="source">	friend class GrassLoader;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_502

 onmouseover="gutterOver(502)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_503

 onmouseover="gutterOver(503)"

><td class="source">	/** \brief Do not create a GrassLayer directly - use GrassLoader-&gt;addLayer() */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_504

 onmouseover="gutterOver(504)"

><td class="source">	GrassLayer(PagedGeometry *geom, GrassLoader *ldr);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_505

 onmouseover="gutterOver(505)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_506

 onmouseover="gutterOver(506)"

><td class="source">	/** \brief Do not delete a GrassLayer yourself - the GrassLoader will do this automatically when it&#39;s deleted */<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_507

 onmouseover="gutterOver(507)"

><td class="source">	~GrassLayer();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_508

 onmouseover="gutterOver(508)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_509

 onmouseover="gutterOver(509)"

><td class="source">	//Updates the vertex shader used by this layer based on the animate enable status<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_510

 onmouseover="gutterOver(510)"

><td class="source">	void _updateShaders();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_511

 onmouseover="gutterOver(511)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_512

 onmouseover="gutterOver(512)"

><td class="source">	//Used by GrassLoader::loadPage() - populates an array with a uniform distribution of grass<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_513

 onmouseover="gutterOver(513)"

><td class="source">	//Returns the final number of grasses, which will always be &lt;= grassCount<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_514

 onmouseover="gutterOver(514)"

><td class="source">	unsigned int _populateGrassList_Uniform(PageInfo page, float *posBuff, unsigned int grassCount);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_515

 onmouseover="gutterOver(515)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_516

 onmouseover="gutterOver(516)"

><td class="source">	//Used by GrassLoader::loadPage() - populates an array of grass positions based on the density map<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_517

 onmouseover="gutterOver(517)"

><td class="source">	//Returns the final number of grasses, which will always be &lt;= grassCount<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_518

 onmouseover="gutterOver(518)"

><td class="source">	unsigned int _populateGrassList_UnfilteredDM(PageInfo page, float *posBuff, unsigned int grassCount);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_519

 onmouseover="gutterOver(519)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_520

 onmouseover="gutterOver(520)"

><td class="source">	//Variation of _populateGrassList(), using bilinear filtering on the density map lookups<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_521

 onmouseover="gutterOver(521)"

><td class="source">	//Returns the final number of grasses, which will always be &lt;= grassCount<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_522

 onmouseover="gutterOver(522)"

><td class="source">	unsigned int _populateGrassList_BilinearDM(PageInfo page, float *posBuff, unsigned int grassCount);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_523

 onmouseover="gutterOver(523)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_524

 onmouseover="gutterOver(524)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_525

 onmouseover="gutterOver(525)"

><td class="source">	GrassLoader *parent;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_526

 onmouseover="gutterOver(526)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_527

 onmouseover="gutterOver(527)"

><td class="source">	//Grass material/shape properties<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_528

 onmouseover="gutterOver(528)"

><td class="source">	Ogre::MaterialPtr material;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_529

 onmouseover="gutterOver(529)"

><td class="source">   Ogre::Real density;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_530

 onmouseover="gutterOver(530)"

><td class="source">	Ogre::Real minWidth, maxWidth;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_531

 onmouseover="gutterOver(531)"

><td class="source">	Ogre::Real minHeight, maxHeight;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_532

 onmouseover="gutterOver(532)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_533

 onmouseover="gutterOver(533)"

><td class="source">	Ogre::Real minY, maxY;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_534

 onmouseover="gutterOver(534)"

><td class="source">   Ogre::Real maxSlope;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_535

 onmouseover="gutterOver(535)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_536

 onmouseover="gutterOver(536)"

><td class="source">	FadeTechnique fadeTechnique;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_537

 onmouseover="gutterOver(537)"

><td class="source">	GrassTechnique renderTechnique;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_538

 onmouseover="gutterOver(538)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_539

 onmouseover="gutterOver(539)"

><td class="source">	//Property maps<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_540

 onmouseover="gutterOver(540)"

><td class="source">	TBounds mapBounds;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_541

 onmouseover="gutterOver(541)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_542

 onmouseover="gutterOver(542)"

><td class="source">	DensityMap *densityMap;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_543

 onmouseover="gutterOver(543)"

><td class="source">	MapFilter densityMapFilter;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_544

 onmouseover="gutterOver(544)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_545

 onmouseover="gutterOver(545)"

><td class="source">	ColorMap *colorMap;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_546

 onmouseover="gutterOver(546)"

><td class="source">	MapFilter colorMapFilter;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_547

 onmouseover="gutterOver(547)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_548

 onmouseover="gutterOver(548)"

><td class="source">	//Grass shader properties<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_549

 onmouseover="gutterOver(549)"

><td class="source">	bool animate, blend, lighting, shaderNeedsUpdate;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_550

 onmouseover="gutterOver(550)"

><td class="source">	Ogre::Real animMag, animSpeed, animFreq;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_551

 onmouseover="gutterOver(551)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_552

 onmouseover="gutterOver(552)"

><td class="source">	//Current frame of animation for this layer<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_553

 onmouseover="gutterOver(553)"

><td class="source">	Ogre::Real waveCount;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_554

 onmouseover="gutterOver(554)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_555

 onmouseover="gutterOver(555)"

><td class="source">	PagedGeometry *geom;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_556

 onmouseover="gutterOver(556)"

><td class="source">};<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_557

 onmouseover="gutterOver(557)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_558

 onmouseover="gutterOver(558)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_559

 onmouseover="gutterOver(559)"

><td class="source">/** \brief A custom page type designed specifically for use with GrassLoader.<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_560

 onmouseover="gutterOver(560)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_561

 onmouseover="gutterOver(561)"

><td class="source">You can use this in your own project if you want, but remember that no optimizations<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_562

 onmouseover="gutterOver(562)"

><td class="source">are performed. The given entity is simply cloned and attached to a new scene node as<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_563

 onmouseover="gutterOver(563)"

><td class="source">quickly and simply as possible (this means there&#39;s no batching overhead as in BatchPage,<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_564

 onmouseover="gutterOver(564)"

><td class="source">but it also means potentially poor performance if you don&#39;t know what you&#39;re doing).<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_565

 onmouseover="gutterOver(565)"

><td class="source">*/<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_566

 onmouseover="gutterOver(566)"

><td class="source">class GrassPage: public GeometryPage<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_567

 onmouseover="gutterOver(567)"

><td class="source">{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_568

 onmouseover="gutterOver(568)"

><td class="source">public:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_569

 onmouseover="gutterOver(569)"

><td class="source">	void init(PagedGeometry *geom, const Ogre::Any &amp;data);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_570

 onmouseover="gutterOver(570)"

><td class="source">	~GrassPage();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_571

 onmouseover="gutterOver(571)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_572

 onmouseover="gutterOver(572)"

><td class="source">	void addEntity(Ogre::Entity *ent, const Ogre::Vector3 &amp;position, const Ogre::Quaternion &amp;rotation, const Ogre::Vector3 &amp;scale, const Ogre::ColourValue &amp;color);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_573

 onmouseover="gutterOver(573)"

><td class="source">	void removeEntities();<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_574

 onmouseover="gutterOver(574)"

><td class="source">	void setFade(bool enabled, Ogre::Real visibleDist, Ogre::Real invisibleDist) {}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_575

 onmouseover="gutterOver(575)"

><td class="source">	void setVisible(bool visible);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_576

 onmouseover="gutterOver(576)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_577

 onmouseover="gutterOver(577)"

><td class="source">private:<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_578

 onmouseover="gutterOver(578)"

><td class="source">	Ogre::SceneManager *sceneMgr;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_579

 onmouseover="gutterOver(579)"

><td class="source">	Ogre::SceneNode *rootNode;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_580

 onmouseover="gutterOver(580)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_581

 onmouseover="gutterOver(581)"

><td class="source">	std::list&lt;Ogre::SceneNode*&gt; nodeList;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_582

 onmouseover="gutterOver(582)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_583

 onmouseover="gutterOver(583)"

><td class="source">	static unsigned long GUID;<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_584

 onmouseover="gutterOver(584)"

><td class="source">	static inline Ogre::String getUniqueID()<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_585

 onmouseover="gutterOver(585)"

><td class="source">	{<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_586

 onmouseover="gutterOver(586)"

><td class="source">		return &quot;GrassPage&quot; + Ogre::StringConverter::toString(++GUID);<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_587

 onmouseover="gutterOver(587)"

><td class="source">	}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_588

 onmouseover="gutterOver(588)"

><td class="source">};<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_589

 onmouseover="gutterOver(589)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_590

 onmouseover="gutterOver(590)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_591

 onmouseover="gutterOver(591)"

><td class="source">}<br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_592

 onmouseover="gutterOver(592)"

><td class="source"><br></td></tr
><tr
id=sl_svn06de7ef9f7907b57123b0e03476e547a1db6ae08_593

 onmouseover="gutterOver(593)"

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
 <a href="/p/ogre-paged/source/detail?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&amp;r=8bbc960029af74335b963770a933d78e564a859d">8bbc960029af</a>
 by m2codeGEN (SVA)
 on May 25, 2011
 &nbsp; <a href="/p/ogre-paged/source/diff?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=8bbc960029af74335b963770a933d78e564a859d&amp;format=side&amp;path=/include/GrassLoader.h&amp;old_path=/include/GrassLoader.h&amp;old=39f8aaf4be3c0602d85bd3fda0f99008c2ef1bef">Diff</a>
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
 
 var selected_path = '/include/GrassLoader.h';
 
 
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
 selected="selected"
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
 <a href="/p/ogre-paged/source/detail?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=39f8aaf4be3c0602d85bd3fda0f99008c2ef1bef">39f8aaf4be3c</a>
 by rortom
 on Apr 9, 2010
 &nbsp; <a href="/p/ogre-paged/source/diff?spec=svn06de7ef9f7907b57123b0e03476e547a1db6ae08&r=39f8aaf4be3c0602d85bd3fda0f99008c2ef1bef&amp;format=side&amp;path=/include/GrassLoader.h&amp;old_path=/include/GrassLoader.h&amp;old=">Diff</a>
 <br>
 <pre class="ifOpened">added svn files</pre>
 </div>
 
 
 <a href="/p/ogre-paged/source/list?path=/include/GrassLoader.h&r=8bbc960029af74335b963770a933d78e564a859d">All revisions of this file</a>
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
 
 <div>Size: 27496 bytes,
 593 lines</div>
 
 <div><a href="//ogre-paged.googlecode.com/hg/include/GrassLoader.h">View raw file</a></div>
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
 var paths = {'svn06de7ef9f7907b57123b0e03476e547a1db6ae08': '/include/GrassLoader.h'}
 codereviews = CR_controller.setup(
 {"projectName":"ogre-paged","assetVersionPath":"http://www.gstatic.com/codesite/ph/15170358673760952803","projectHomeUrl":"/p/ogre-paged","token":"oiJKzYbWMYHYNnIKb0LF_0MVpho:1365060155036","relativeBaseUrl":"","loggedInUserEmail":"GeoffTopping@gmail.com","profileUrl":"/u/107319084838887332900/","domainName":null,"assetHostPath":"http://www.gstatic.com/codesite/ph"}, '', 'svn06de7ef9f7907b57123b0e03476e547a1db6ae08', paths,
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

