#ifndef _SYNTAX_HTML_H
#define _SYNTAX_HTML_H

//html
char *HTML_extensions[] = {".html", ".htm", NULL};
char *HTML_keywords[] = {
  //scrapped from www.fillster.com/htmlcodes/ using python lxml and requests
  //then c processing
  //start tokens:
  "<a>", "<abbr>", "<acronym>", "<address>", "<applet>", "<area>", "<b>",
  "<base>", "<basefont>", "<bdo>", "<bgsound>", "<big>", "<blockquote>",
  "<blink>", "<body>", "<br>", "<button>", "<caption>", "<center>", "<cite>",
  "<code>", "<col>", "<colgroup>", "<dd>", "<dfn>", "<del>", "<dir>", "<dl>",
  "<div>", "<dt>", "<embed>", "<em>", "<fieldset>", "<font>", "<form>",
  "<frame>", "<frameset>", "<h1> - <h6>", "<head>", "<hr>", "<html>",
  "<iframe>", "<img>", "<input>", "<ins>" , "<isindex>", "<i>", "<kbd>",
  "<label>", "<legend>", "<li>", "<link>", "<marquee>", "<menu>", "<meta>",
  "<noframe>", "<noscript>", "<optgroup>", "<option>", "<ol>", "<p>", "<pre>",
  "<q>", "<s>", "<samp>", "<script>", "<select>", "<small>", "<span>",
  "<strike>", "<strong>", "<style>", "<sub>", "<sup>", "<table>", "<td>",
  "<th>", "<tr>", "<tbody>", "<textarea>", "<tfoot>", "<thead>", "<title>",
  "<tt>", "<u>", "<ul>", "<var>",
  //end tokens:
  "</a>", "</abbr>", "</acronym>", "</address>", "</applet>", "</area>",
  "</b>", "</base>", "</basefont>", "</bdo>", "</bgsound>", "</big>",
  "</blockquote>", "</blink>", "</body>", "</br>", "</button>",
  "</caption>", "</center>", "</cite>", "</code>", "</col>", "</colgroup>",
  "</dd>", "</dfn>", "</del>", "</dir>", "</dl>", "</div>", "</dt>",
  "</embed>", "</em>", "</fieldset>", "</font>", "</form>", "</frame>",
  "</frameset>", "</h1> - <h6>", "</head>", "</hr>", "</html>", "</iframe>",
  "</img>", "</input>", "</ins>", "</isindex>", "</i>", "</kbd>",
  "</label>", "</legend>", "</li>", "</link>", "</marquee>", "</menu>",
  "</meta>", "</noframe>", "</noscript>", "</optgroup>", "</option>",
  "</ol>", "</p>", "</pre>", "</q>", "</s>", "</samp>", "</script>",
  "</select>", "</small>", "</span>", "</strike>", "</strong>", "</style>",
  "</sub>", "</sup>", "</table>", "</td>", "</th>", "</tr>", "</tbody>",
  "</textarea>", "</tfoot>", "</thead>", "</title>", "</tt>", "</u>",
  "</ul>", "</var>",
  //start tokens (HTML5 only):
  "<article>", "<aside>", "<bdi>", "<details>", "<dialog>", "<figcaption>",
  "<figure>", "<footer>", "<header>", "<main>", "<mark>", "<menuitem>",
  "<meter>", "<nav>", "<progress>", "<rp>", "<rt>", "<ruby>", "<section>",
  "<summary>", "<time>", "<wbr>",
  //end tokens (HTML5 only):
  "</article>", "</aside>", "</bdi>", "</details>", "</dialog>",
  "</figcaption>", "</figure>", "</footer>", "</header>", "</main>", "</mark>",
  "</menuitem>", "</meter>", "</nav>", "</progress>", "</rp>", "</rt>",
  "</ruby>", "</section>", "</summary>", "</time>", "</wbr>"
};

#define HTML_syntax { \
	HTML_extensions, \
	HTML_keywords, \
	"//", \
	"<!--", \
	"-->", \
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
