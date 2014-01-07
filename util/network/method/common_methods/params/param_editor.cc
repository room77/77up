// Copyright 2013 Room77 Inc. All Rights Reserved.
// Author: pramodg@room77.com (Pramod Gupta)


// TODO(pramodg): Write a method to allow access to its functions.

#include "util/network/method/common_methods/params/param_editor.h"

#include <sstream>

#include "base/args/args.h"
#include "base/strutil.h"

namespace params {

using args::ArgInfo;

namespace {

void AddArgToEditForm(int count, const ArgInfo *info,
                                 stringstream *output) {
  string name_escaped = strutil::EscapeString_HTML(info->name());
  string value_escaped = strutil::EscapeString_HTML(info->Value());

  string cell_dom = "<td colspan=2 align=center><i>*hidden*</i></td>";

  if (info->editable()) {
    stringstream ss;
    ss << "<td><span class='restore'><input type='button' id='restore" << count
       << "' value='Restore'></span></td><td><input type='text' id='cell"
       << count << "' size=20 value='" << value_escaped << "'></td>";
    cell_dom = ss.str();
  }

  (*output)
    << "<tr class='row"
    << (count % 2 == 0 ? " even" : " odd") << (info->edited() ? " edited" : "")
    << "' count='" << count << "'><td>"
    << "<span id='name" << count << "'>" << name_escaped
    << "</span></td><td>"
    << strutil::EscapeString_HTML(info->ArgType()) << "</td><td>"
    << strutil::EscapeString_HTML(info->description()) << "</td>"
    << cell_dom
    << "</tr>";
}

}  // namespace

// command-line argument display / edit interface (output in HTML)
string ParamEditor() {
  static const string jq_code = "\
  <link href='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/themes/base/jquery-ui.css' rel='stylesheet' type='text/css'/> \
  <script src='http://ajax.googleapis.com/ajax/libs/jquery/1.5/jquery.min.js'></script> \
  <script src='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8/jquery-ui.min.js'></script> \
  <script src='http://jquery-json.googlecode.com/files/jquery.json-2.4.js'></script> \
  <script> \
  function UpdateParam(name, value, $row, $input) { \
    if (!($row.hasClass('edited'))) return; \
    $('#msg').empty(); \
    var d = { 'name': name }; \
    if (value != null) d['value'] = value; \
    else d['restore'] = true; \
    $.ajax({ \
      url: '/.edit_param', \
      dataType: 'json', \
      data: { 'q': $.toJSON(d) }, \
      success: function(data, textStatus, xhr) { \
        if (data.error_msg) $('#msg').text(data.error_msg); \
        else { \
          $('#msg').text(data.message); \
          if (data.restored) { \
            $row.removeClass('edited'); \
            $input.val(data.value); \
          } \
        } \
      }, \
      error: function() { \
        $('#msg').text('Error communicating with server.'); \
      }, \
      cache: false \
    }); \
  } \
  \
  function InitEventHandlers($row, name, $input, $restore) { \
    $input.change(function() { \
      $row.addClass('edited'); \
      UpdateParam(name, $input.val(), $row, $input); \
    }).keydown(function() { \
      $('#msg').empty(); \
    }); \
    $restore.click(function() { \
      UpdateParam(name, null, $row, $input); \
    }); \
  } \
  \
  $(document).ready(function() { \
    $('.row').each(function() { \
      var count = $(this).attr('count'); \
      var name = $('#name' + count).text(); \
      var $input = $('#cell' + count); \
      var $restore = $('#restore' + count); \
      InitEventHandlers($(this), name, $input, $restore); \
      $('#msg').click(function() { \
        $('#msg').empty(); \
      }); \
    }); \
  }); \
  </script> \
  ";

  stringstream html;
  html << "<html><head><title>Parameter Editor - R77 RPC Server</title>"
       << "<style type='text/css'>\n"
       << "td { font-size: 10pt; }\n"
       << ".even { background-color:#DDDDDD; }\n"
       << ".odd { background-color:#EEEEEE; }\n"
       << ".edited { background-color:#BCEE68; }\n"
       << ".edited .restore { visibility:visible; }\n"
       << ".restore { visibility:hidden; }\n"
       << ".first { background-color: #C0C0C0; text-align: center; border-bottom: 1pm dashed; }"
       << "#msg { position:fixed; top:10px; right: 20px; background-color: #BCEE68; }\n"
       << "</style>\n"
       << "<link rel=\"icon\" type=\"image/png\" "
       << "href=\"/images/icons/fav-internal.png\" />\n"
       << jq_code

       << "</head><body><h3>Parameter Editor</h3><div id='msg'></div>"
       << "<table border=0><thead><tr class='first'>"
       << "<th>Name</th><th>Type</th><th>Description</th>"
       << "<th colspan=3>Value</th></tr></thead></tbody>\n";

  const vector<const ArgInfo*>& args_sorted =
      args::CommandLineArgs::Instance().GetArgsSorted();
  for (int i = 0; i < args_sorted.size(); ++i) {
    const ArgInfo *info = args_sorted[i];
    AddArgToEditForm(i + 1, info, &html);
  }

  html << "</tbody></table></body></html>\n";

  return html.str();
}

}  // namespace params
