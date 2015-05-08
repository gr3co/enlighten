//
//  ResultView.h
//  Enlighten
//
//  Created by Stephen Greco on 5/7/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol ResultViewDelegate <NSObject>

@required
- (void) resultViewDidClose:(UIView*)view;

@end

@interface ResultView : UIView <UIWebViewDelegate>

-(id) initWithFrame:(CGRect)frame
             result:(int)result
        andDelegate:(id<ResultViewDelegate>) delegate;

@property UIWebView *webView;
@property UILabel *infoView;
@property UIButton *closeWebViewButton;
@property int result;
@property id<ResultViewDelegate> delegate;

@end
