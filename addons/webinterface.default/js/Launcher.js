/*
 *      Copyright (C) 2005-2011 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

/* This launcher is based on the scriptaculous.js launch script */

// Copyright (c) 2005-2008 Thomas Fuchs (http://script.aculo.us, http://mir.aculo.us)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// For details, see the script.aculo.us web site: http://script.aculo.us/

(function()
{
  var DEBUG_MODE = false; /* Set to true to disable cached javascript */

  var Launcher = {
    VERSION: '1.0.0',
    JS_REGEX: /Launcher\.js(\?.*)?$/,
    load: function(libraryName) {
      document.write('<script type="text/javascript" src="' + libraryName + '?' + (DEBUG_MODE ? Math.random() : this.VERSION) + '"><\/script>');
    },
    init: function() {
      var i, j, s, includes, load, path, scripts;
      scripts = window.document.getElementsByTagName('script');
      for (i = 0; i < scripts.length; i += 1) {
        s = scripts[i];
        if (s.src.match(this.JS_REGEX)) {
          path = s.src.replace(this.JS_REGEX, '');
          load = s.src.match(/\?.*load=([a-zA-Z0-9.,\-]*)/);
          includes = (load && load[1] || '').split(',');
          for (j = 0; j < includes.length; j += 1) {
            Launcher.load(path + includes[j] + '.js');
          }
        }
      }
    }
  };

  Launcher.init();
})();
