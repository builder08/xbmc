/*
 *      Copyright (C) 2012-present Team Kodi
 *      This file is part of Kodi - https://kodi.tv
 *
 *  Kodi is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Kodi is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#import <UIKit/UIKit.h>
#include "IOSKeyboard.h"

@interface KeyboardView : UIView <UITextFieldDelegate>
{
  NSMutableString *text;
  BOOL _confirmed;
  CIOSKeyboard *_iosKeyboard;
  bool *_canceled;
  BOOL _deactivated;
  UITextField *_textField;
  UITextField *_heading;
  int _keyboardIsShowing; // 0: not, 1: will show, 2: showing
  CGRect _kbRect;
}

@property (nonatomic, retain) NSMutableString *text;
@property (getter = isConfirmed) BOOL _confirmed;
@property (assign, setter = registerKeyboard:) CIOSKeyboard *_iosKeyboard;
@property CGRect _frame;

- (void) setHeading:(NSString *)heading;
- (void) setHidden:(BOOL)hidden;
- (void) activate;
- (void) deactivate;
- (void) setKeyboardText:(NSString*)aText closeKeyboard:(BOOL)closeKeyboard;
- (void) textChanged:(NSNotification*)aNotification;
- (void) setCancelFlag:(bool *)cancelFlag;
- (void) doDeactivate:(NSDictionary *)dict;
- (id)initWithFrameInternal;
@end
