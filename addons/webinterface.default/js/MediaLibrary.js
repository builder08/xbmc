/*
 *      Copyright (C) 2005-2010 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

var MediaLibrary = function() {
		this.init();
		return true;
	}

MediaLibrary.prototype = {
		init: function() {
			this.bindControls();
		},
		bindControls: function() {
			$('#musicLibrary').click(jQuery.proxy(this.musicLibraryOpen, this));
			$('#movieLibrary').click(jQuery.proxy(this.movieLibraryOpen, this));
			$('#tvshowLibrary').click(jQuery.proxy(this.tvshowLibraryOpen, this));
		},
		musicLibraryOpen: function(event) {
			$('#musicLibrary').addClass('selected');
			$('#movieLibrary').removeClass('selected');
			$('#tvshowLibrary').removeClass('selected');
			$('.contentContainer').css('z-index', 1);
			var libraryContainer = $('#libraryContainer');
			if (!libraryContainer || libraryContainer.length == 0) {
				$('#spinner').show();
				jQuery.post(JSON_RPC + '?GetAlbums', '{"jsonrpc": "2.0", "method": "AudioLibrary.GetAlbums", "params": { "start": 0, "fields": ["album_description", "album_theme", "album_mood", "album_style", "album_type", "album_label", "album_artist", "album_genre", "album_rating", "album_title"] }, "id": 1}', jQuery.proxy(function(data) {
					if (data && data.result && data.result.albums) {
							libraryContainer = $('<div>');
							libraryContainer.css('z-index', 100)
											.attr('id', 'libraryContainer')
											.addClass('contentContainer');
							$('#content').append(libraryContainer);
					} else {
						libraryContainer.html('');
					}
					$.each($(data.result.albums), jQuery.proxy(function(i, item) {
						var floatableAlbum = this.generateThumb('album', item.thumbnail, item.album_title, item.album_artist);
						floatableAlbum.bind('click', { album: item }, jQuery.proxy(this.displayAlbumDetails, this));
						libraryContainer.append(floatableAlbum);
					}, this));
					$('#spinner').hide();
					//$('#libraryContainer img').lazyload();
				}, this), 'json');
			} else {
				libraryContainer.css('z-index', 100);
			}
		},
		generateThumb: function(type, thumbnail, album_title, album_artist) {
			var floatableAlbum = $('<div>');
			var path = thumbnail ? ('/vfs/' + thumbnail) : DEFAULT_ALBUM_COVER;
			var title = album_title;
			var artist = album_artist;
			if (title.length > 18 && !(title.length <= 21)) {
				title = album_title.substring(0, 18) + '...';
			}
			if (artist.length > 20 && !(artist.length <= 22)) {
				artist = album_artist.substring(0, 20) + '...';
			}
			var className = '';
			var code = '';
			switch(type) {
				case 'album':
					className = 'floatableAlbum';
					code = '<p class="album" title="' + album_title + '">' + title + '</p><p class="artist" title="' + album_artist + '">' + artist + '</p>';
					break;
				case 'movie':
					className = 'floatableMovieCover';
					code = '<p class="album" title="' + album_title + '">' + title + '</p>';
					break;
				case 'tvshow':
					className = 'floatableTVShowCover';
					break;
			}
			floatableAlbum.addClass(className).html('<div class="imgWrapper"><div class="inner"><img src="' + path + '" alt="" /></div></div>' + code);
			return floatableAlbum;
		},
		displayAlbumDetails: function(event) {
			var albumDetailsContainer = $('#albumDetails' + event.data.album.albumid);
			if (!albumDetailsContainer || albumDetailsContainer.length == 0) {
				$('#spinner').show();
				jQuery.post(JSON_RPC + '?GetSongs', '{"jsonrpc": "2.0", "method": "AudioLibrary.GetSongs", "params": { "fields": ["title", "artist", "genre", "tracknumber", "discnumber", "duration", "year"], "albumid" : ' + event.data.album.albumid + ' }, "id": 1}', jQuery.proxy(function(data) {
					albumDetailsContainer = $('<div>');
					albumDetailsContainer.attr('id', 'albumDetails' + event.data.album.albumid)
										 .addClass('contentContainer')
										 .css('z-index', 100)
										 .html('<table class="albumView"><tr><th>Artwork</th><th>&nbsp;</th><th>Name</th><th>Time</th><th>Artist</th><th>Genre</th></tr><tbody class="resultSet"></tbody></table>');
					$('#content').append(albumDetailsContainer);
					var albumThumbnail = event.data.album.thumbnail;
					var albumTitle = event.data.album.album_title||'Unknown';
					var albumArtist = event.data.album.album_artist||'Unknown';
					var trackCount = data.result.total;
					$.each($(data.result.songs), jQuery.proxy(function(i, item) {
						var trackRow = $('<tr>');
						if (i == 0) {
							var albumTD = $('<td>');
							albumTD.attr('rowspan', ++trackCount).addClass('albumThumb');
							trackRow.append(albumTD);
						}
						var trackNumberTD = $('<td>');
						trackNumberTD.html(item.tracknumber).addClass('track').bind('click', { song: item, album: event.data.album }, jQuery.proxy(this.playTrack, this));
						trackRow.append(trackNumberTD);
						var trackTitleTD = $('<td>');
						trackTitleTD.html(item.title).addClass('track').bind('click', { song: item, album: event.data.album }, jQuery.proxy(this.playTrack, this));
						trackRow.append(trackTitleTD);
						var trackDurationTD = $('<td>');
						trackDurationTD.html(durationToString(item.duration)).addClass('track').bind('click', { song: item, album: event.data.album }, jQuery.proxy(this.playTrack, this));
						trackRow.append(trackDurationTD);
						var trackArtistTD = $('<td>');
						trackArtistTD.html(item.artist).addClass('track').bind('click', { song: item, album: event.data.album }, jQuery.proxy(this.playTrack, this));
						trackRow.append(trackArtistTD);
						var trackGenreTD = $('<td>');
						trackGenreTD.html(item.genre).addClass('track').bind('click', { song: item, album: event.data.album }, jQuery.proxy(this.playTrack, this));
						trackRow.append(trackGenreTD);
						$('#albumDetails' + event.data.album.albumid + ' .resultSet').append(trackRow);
					}, this));
					if (trackCount > 0) {
						var trackRow = $('<tr>');
						var trackNumberTD = $('<td>');
						trackNumberTD.addClass('fillerTrack').html('&nbsp');
						trackRow.append(trackNumberTD);
						var trackTitleTD = $('<td>');
						trackTitleTD.addClass('fillerTrack').html('&nbsp');
						trackRow.append(trackTitleTD);
						var trackDurationTD = $('<td>');
						trackDurationTD.addClass('fillerTrack').html('&nbsp');
						trackRow.append(trackDurationTD);
						var trackArtistTD = $('<td>');
						trackArtistTD.addClass('fillerTrack').html('&nbsp');
						trackRow.append(trackArtistTD);
						var trackGenreTD = $('<td>');
						trackGenreTD.addClass('fillerTrack').html('&nbsp');
						trackRow.append(trackGenreTD);
						$('#albumDetails' + event.data.album.albumid + ' .resultSet').append(trackRow);
					}
					$('#albumDetails' + event.data.album.albumid + ' .albumThumb').append(this.generateThumb('album', albumThumbnail, albumTitle, albumArtist));
					$('.contentContainer').css('z-index', 1);
					$('#spinner').hide();
				}, this), 'json');
			} else {
				$('.contentContainer').css('z-index', 1);
				$('#albumDetails' + event.data.album.albumid).css('z-index', 100);
			}
		},
		displayMovieDetails: function(event) {
			
		},
		displayTVShowDetails: function(event) {

		},
		playTrack: function(event) {
			jQuery.post(JSON_RPC + '?ClearPlaylist', '{"jsonrpc": "2.0", "method": "AudioPlaylist.Clear", "id": 1}', jQuery.proxy(function(data) {
				//check that clear worked.
				jQuery.post(JSON_RPC + '?AddAlbumToPlaylist', '{"jsonrpc": "2.0", "method": "AudioPlaylist.Add", "params": { "albumid": ' + event.data.album.albumid + ' }, "id": 1}', jQuery.proxy(function(data) {
					//play specific song in playlist
					jQuery.post(JSON_RPC + '?PlaylistItemPlay', '{"jsonrpc": "2.0", "method": "AudioPlaylist.Play", "params": { "songid": ' + event.data.song.songid + ' }, "id": 1}', function() {}, 'json');
				}, this), 'json');
			}, this), 'json');
		},
		movieLibraryOpen: function() {
			$('#musicLibrary').removeClass('selected');
			$('#tvshowLibrary').removeClass('selected');
			$('#movieLibrary').addClass('selected');
			$('.contentContainer').css('z-index', 1);
			var libraryContainer = $('#movieLibraryContainer');
			if (!libraryContainer || libraryContainer.length == 0) {
				$('#spinner').show();
				jQuery.post(JSON_RPC + '?GetMovies', '{"jsonrpc": "2.0", "method": "VideoLibrary.GetMovies", "params": { "start": 0, "fields": ["genre", "director", "trailer", "tagline", "plot", "plotoutline", "title", "originaltitle", "lastplayed", "showtitle", "firstaired", "duration", "season", "episode", "runtime", "year", "playcount", "rating"] }, "id": 1}', jQuery.proxy(function(data) {
					if (data && data.result && data.result.movies) {
							libraryContainer = $('<div>');
							libraryContainer.css('z-index', 100)
											.attr('id', 'movieLibraryContainer')
											.addClass('contentContainer');
							$('#content').append(libraryContainer);
					} else {
						libraryContainer.html('');
					}
					$.each($(data.result.movies), jQuery.proxy(function(i, item) {
						var floatableMovieCover = this.generateThumb('movie', item.thumbnail, item.title, "");
						floatableMovieCover.bind('click', { movie: item }, jQuery.proxy(this.displayMovieDetails, this));
						libraryContainer.append(floatableMovieCover);
					}, this));
					$('#spinner').hide();
					//$('#libraryContainer img').lazyload();
				}, this), 'json');
			} else {
				libraryContainer.css('z-index', 100);
			}
		},
		tvshowLibraryOpen: function() {
			$('#musicLibrary').removeClass('selected');
			$('#tvshowLibrary').addClass('selected');
			$('#movieLibrary').removeClass('selected');
			$('.contentContainer').css('z-index', 1);
			var libraryContainer = $('#tvshowLibraryContainer');
			if (!libraryContainer || libraryContainer.length == 0) {
				$('#spinner').show();
				jQuery.post(JSON_RPC + '?GetTVShows', '{"jsonrpc": "2.0", "method": "VideoLibrary.GetTVShows", "params": { "start": 0, "fields": ["genre", "director", "trailer", "tagline", "plot", "plotoutline", "title", "originaltitle", "lastplayed", "showtitle", "firstaired", "duration", "season", "episode", "runtime", "year", "playcount", "rating"] }, "id": 1}', jQuery.proxy(function(data) {
					if (data && data.result && data.result.tvshows) {
							libraryContainer = $('<div>');
							libraryContainer.css('z-index', 100)
											.attr('id', 'tvshowLibraryContainer')
											.addClass('contentContainer');
							$('#content').append(libraryContainer);
					} else {
						libraryContainer.html('');
					}
					$.each($(data.result.tvshows), jQuery.proxy(function(i, item) {
						var floatableTVShowCover = this.generateThumb('tvshow', item.thumbnail, item.title, "");
						floatableTVShowCover.bind('click', { tvshow: item }, jQuery.proxy(this.displayTVShowDetails, this));
						libraryContainer.append(floatableTVShowCover);
					}, this));
					//$('#libraryContainer img').lazyload();
					$('#spinner').hide();
				}, this), 'json');
			} else {
				libraryContainer.css('z-index', 100);
			}
		}
	}