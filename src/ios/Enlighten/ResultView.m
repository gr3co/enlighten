//
//  ResultView.m
//  Enlighten
//
//  Created by Stephen Greco on 5/7/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import "ResultView.h"

static NSString *page_1_url = @"http://en.m.wikipedia.org/wiki/Mona_lisa";
static NSString *page_2_url = @"http://en.m.wikipedia.org/wiki/Yellowstone_National_Park";
static NSString *page_3_url = @"http://www.ece.cmu.edu/~ece549/spring15/team10/website";
static NSString *rick_roll_url = @"https://www.youtube.com/watch?v=dQw4w9WgXcQ";

@implementation ResultView

-(id) initWithFrame:(CGRect)frame
             result:(int)result
        andDelegate:(id<ResultViewDelegate>)delegate {
    if (self = [super initWithFrame:frame]) {
        self.backgroundColor = [UIColor colorWithRed:0xf7/255.0
                                               green:0xf4/255.0
                                                blue:7/255.0
                                               alpha:1.0];
        _result = result;
        _delegate = delegate;
        [self setupButton];
        [self setupInfoView];
        [self setupWebView];
    }
    return self;
}

- (void) setupButton {
    _closeWebViewButton = [[UIButton alloc] initWithFrame:CGRectMake(self.frame.size.width / 2 - 50,
                                                                     70, 100, 20)];
    [_closeWebViewButton setBackgroundColor:[UIColor whiteColor]];
    [_closeWebViewButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    [_closeWebViewButton setTitle:@"close" forState:UIControlStateNormal];
    [_closeWebViewButton addTarget:self action:@selector(close) forControlEvents:UIControlEventTouchUpInside];
    [self addSubview:_closeWebViewButton];
}

- (void) setupInfoView {
    float width = self.frame.size.width;
    _infoView = [[UILabel alloc] initWithFrame:
                   CGRectMake(0.1 * width,
                              0, 0.8*width, 100)];
    _infoView.backgroundColor = [UIColor clearColor];
    _infoView.textColor = [UIColor blueColor];
    _infoView.text = [NSString stringWithFormat: @"Found code: 0x%02X", _result];
    _infoView.textAlignment = NSTextAlignmentCenter;
    _infoView.numberOfLines = 0;
    _infoView.font = [UIFont fontWithName:@"Menlo-Bold" size:24];
    [self addSubview:_infoView];
}

- (void) setupWebView {
    _webView = [[UIWebView alloc] initWithFrame:CGRectMake(0, 100, self.frame.size.width,
                                                           self.frame.size.height - 100)];
    _webView.delegate = self;
    NSURL *url;
    switch (_result) {
        case 0xab: url = [NSURL URLWithString:page_1_url]; break;
        case 0xcd: url = [NSURL URLWithString:page_2_url]; break;
        case 0xef: url = [NSURL URLWithString:page_3_url]; break;
        default: url = [NSURL URLWithString:rick_roll_url];
    }
    [self addSubview:_webView];
    [_webView loadRequest:[NSURLRequest requestWithURL:url]];
}

- (void) close {
    [_delegate resultViewDidClose:self];
    [self removeFromSuperview];
}

@end
