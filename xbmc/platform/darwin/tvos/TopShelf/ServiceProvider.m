/*
 *      Copyright (C) 2015 Team MrMC
 *      https://github.com/MrMC
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
 *  along with MrMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#import "ServiceProvider.h"

#import "../tvosShared.h"

@implementation ServiceProvider

#pragma mark - TVTopShelfProvider protocol

- (TVTopShelfContentStyle)topShelfStyle
{
  return TVTopShelfContentStyleSectioned;
}

- (NSArray<TVContentItem*>*)topShelfItems
{
  __auto_type storeUrl = [tvosShared getSharedURL];
  if (!storeUrl)
    return @[];

  __auto_type const sharedID = [tvosShared getSharedID];
  __auto_type const shared = [[NSUserDefaults alloc] initWithSuiteName:sharedID];
  __auto_type topShelfItems = [[NSMutableArray alloc] init];
  __auto_type wrapperIdentifier =
      [[TVContentIdentifier alloc] initWithIdentifier:@"shelf-wrapper" container:nil];

  NSArray* moviesArray = nil;
  NSArray* tvshowsArray = nil;
  NSArray* tvchannelsArray = nil;
  NSDictionary* sharedDict = nil;

  if ([tvosShared isJailbroken])
  {
    __auto_type sharedDictUrl =
        [storeUrl URLByAppendingPathComponent:@"shared.dict" isDirectory:NO];
    sharedDict = [NSDictionary dictionaryWithContentsOfFile:[sharedDictUrl path]];

    moviesArray = [sharedDict valueForKey:@"movies"];
    tvshowsArray = [sharedDict valueForKey:@"tvshows"];
    tvchannelsArray = [sharedDict valueForKey:@"tvchannels"];
  }
  else
  {
    moviesArray = [shared objectForKey:@"movies"];
    tvshowsArray = [shared valueForKey:@"tvshows"];
    tvchannelsArray = [shared valueForKey:@"tvchannels"];
  }

  __auto_type mainAppBundle = [tvosShared mainAppBundle];
  __auto_type kodiUrlScheme = @"kodi"; // fallback value
  NSDictionary* dic;
  for (dic in [mainAppBundle objectForInfoDictionaryKey:@"CFBundleURLTypes"])
  {
    if ([dic[@"CFBundleURLName"] isEqualToString:mainAppBundle.bundleIdentifier])
    {
      kodiUrlScheme = dic[@"CFBundleURLSchemes"][0];
      break;
    }
  }

  storeUrl = [storeUrl URLByAppendingPathComponent:@"RA" isDirectory:YES];
  __auto_type contentItemsFrom = ^NSArray<TVContentItem*>*(NSArray* videosArray)
  {
    NSMutableArray<TVContentItem*>* contentItems =
        [[NSMutableArray alloc] initWithCapacity:videosArray.count];
    NSDictionary* videoDict;
    for (videoDict in videosArray)
    {
      __auto_type identifier =
          [[TVContentIdentifier alloc] initWithIdentifier:@"VOD" container:wrapperIdentifier];
      __auto_type contentItem = [[TVContentItem alloc] initWithContentIdentifier:identifier];

      [contentItem
          setImageURL:[storeUrl URLByAppendingPathComponent:[videoDict valueForKey:@"thumb"]
                                                isDirectory:NO]
            forTraits:TVContentItemImageTraitScreenScale1x];
      contentItem.imageShape = TVContentItemImageShapePoster;
      contentItem.title = [videoDict valueForKey:@"title"];
      NSString* url = [videoDict valueForKey:@"url"];
      contentItem.displayURL = [NSURL
          URLWithString:[NSString stringWithFormat:@"%@://display/movie/%@", kodiUrlScheme, url]];
      contentItem.playURL = [NSURL
          URLWithString:[NSString stringWithFormat:@"%@://play/movie/%@", kodiUrlScheme, url]];
      [contentItems addObject:contentItem];
    }
    return contentItems;
  };

  if([moviesArray count] > 0 || [tvshowsArray count] > 0)
  {
    if ([moviesArray count] > 0)
    {
      __auto_type itemMovie = [[TVContentItem alloc] initWithContentIdentifier:wrapperIdentifier];
      itemMovie.title = [(sharedDict ?: shared) valueForKey:@"moviesTitle"];
      itemMovie.topShelfItems = contentItemsFrom(moviesArray);
      [topShelfItems addObject:itemMovie];
    }

    if ([tvshowsArray count] > 0)
    {
      __auto_type itemTvshow = [[TVContentItem alloc] initWithContentIdentifier:wrapperIdentifier];
      itemTvshow.title = [(sharedDict ?: shared) valueForKey:@"tvshowsTitle"];
      itemTvshow.topShelfItems = contentItemsFrom(tvshowsArray);
      [topShelfItems addObject:itemTvshow];
    }
  }
  
  else if ([tvchannelsArray count] > 0)
  {
    __auto_type itemTvchannel = [[TVContentItem alloc] initWithContentIdentifier:wrapperIdentifier];
    itemTvchannel.title = [(sharedDict ?: shared) valueForKey:@"tvchannelsTitle"];
    itemTvchannel.topShelfItems = contentItemsFrom(tvchannelsArray);
    [topShelfItems addObject:itemTvchannel];
  }

  return topShelfItems;
}

@end
