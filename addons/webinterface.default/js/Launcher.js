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

(function()
{
  var scripts =
  [
    "js/jquery-1.5.2.min.js",
    "js/jquery.lazyload.js",
    "js/iscroll-min.js",
    "js/Core.js",
    "js/MediaLibrary.js",
    "js/NowPlayingManager.js"
  ];
  
  var DEBUG_MODE = false; /* Set to true to disable cached javascript */
  var VERSION = '1.0.0';
  var i;

  for (i = 0; i < scripts.length; i += 1) {
    document.write('<script type="text/javascript" src="' + scripts[i] + '?' + (DEBUG_MODE ? Math.random() : VERSION) + '"><\/script>');
  }
})();
