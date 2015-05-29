#! /usr/bin/env python
#coding:utf-8
import json
import logging
import traceback
import sys
import codecs
import time

reload(sys)
sys.setdefaultencoding('utf-8')
class Tweet(object):
    def __init__(self):
        self.tid = 0
        self.type = 0
        self.f_catalog = '' 
        self.s_catalog = ''
        self.tag = []
        self.zan_num = 0
        self.comment_num = 0
        self.desc = []
        self.score = -1
        self.ctime = 0

def _get_content(imgs):
    content = []
    try:
        imgs = json.loads(imgs)
        for img in imgs:
            c = img.get('content', '')
            if c:
                content.append(c)
    except Exception, e:
        logging.info("get content error, e[%s]", e)

    return content

def _load_tweet(file):
    tweet_dict = {}#tid:tweet
    with open(file) as f:
        logging.info("load tweet from [%s]" % (file))
        for index, line in enumerate(f):
            if index == 0:
                continue
            tweet = Tweet()
            line = line.strip('\n')
            item = line.split('\t')
            tid = int(item[0])
            tweet.tid = tid
            tweet.type = int(item[1])
            tweet.desc = _get_content(item[2])
            tweet.s_catalog = item[3]
            tweet.tag = item[4].split(',')
            tweet.f_catalog = item[5]
            tweet.ctime = item[6]
            if ''.join(tweet.desc) == '' and item[3] == '' and ''.join(tweet.tag) == '' and item[5] == '':
                continue

            tweet_dict[tid] = tweet
    return tweet_dict

def _load_zan(file):
    zan_dict = {} #tid:number
    logging.info('load zan[%s]' % (file))
    with open(file) as f:
        for index, line in enumerate(f):
            if index == 0:
                continue
            line = line.strip('\n')
            item = line.split('\t')
            tid = int(item[0])
            if tid in zan_dict:
                zan_dict[tid] += 1
            else:
                zan_dict[tid] = 1
    return zan_dict 

def _load_comment(file):
    comment_dict = {}#poi_id: biz_area_id
    logging.info('load comment[%s]' % (file)) 
    with open(file) as f:
        for index, line in enumerate(f):
            if index == 0:
                continue
            line = line.strip('\n')
            item = line.split('\t')
            tid = int(item[1])
            if tid in comment_dict:
                comment_dict[tid] += 1
            else:
                comment_dict[tid] = 1
    return comment_dict

def _get_score(tweet):
    G = 1.8
    zan_num = int(tweet.zan_num)
    comment_num = int(tweet.comment_num)
    ctime = int(tweet.ctime)

    t = (time.time() - ctime)/3600.0

    score = (0.6*zan_num + 0.4*comment_num)/(t+2)**G

    return score

def merge_data(out_file, tweet_file, zan_file, comment_file):
    tweet_dict = _load_tweet(tweet_file)
    zan_dict = _load_zan(zan_file)
    comment_dict = _load_comment(comment_file)

    logging.info('total tweet: %d' % (len(tweet_dict)))
    #fp_out = codecs.open(out_file, 'w', "utf-8")
    fp_out = open(out_file, 'w')
    tweet_list = []
    for tid in tweet_dict:
        try:
            tweet = tweet_dict[tid]
            zan_num = zan_dict.get(tid, 0)
            tweet.zan_num = zan_num
            comment_num = comment_dict.get(tid, 0)
            tweet.comment_num = comment_num
            tweet.score = _get_score(tweet)
            tweet_list.append(tweet)
        except Exception,e:
             logging.info(traceback.format_exc())
    tweet_list = sorted(tweet_list, key=lambda x:x.score)

    for tweet in tweet_list:
        #line = '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' % (tweet.tid, tweet.type, tweet.f_catalog, tweet.s_catalog, ' '.join(tweet.tag), tweet.zan_num, tweet.comment_num, tweet.desc)
        info = {}
        info['tid'] = tweet.tid
        info['type'] = tweet.type
        info['f_catalog'] = tweet.f_catalog
        info['s_catalog'] = tweet.s_catalog
        info['tag'] = tweet.tag
        info['zan_num'] = tweet.zan_num
        info['comment_num'] = tweet.comment_num
        info['desc'] = tweet.desc
        line = '%s\n' % json.dumps(info)

        #logging.info('line:%s' % line)
        fp_out.write(line)
    fp_out.close()

def main():
    if 7 != len(sys.argv):
        logging.warning('cmd args error')
        exit(1)
    output_file = sys.argv[1]
    tweet_file = sys.argv[2]
    zan_file = sys.argv[3]
    comment_file = sys.argv[4]
    logging.info('merge_data: output[%s] tweet[%s] zan[%s] comment[%s]' % (output_file, tweet_file, zan_file, comment_file))
    try:
        merge_data(output_file, tweet_file, zan_file, comment_file)
    except Exception as e:
        logging.warning(traceback.format_exc())
        exit(1)


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO,
                        format='%(asctime)s %(levelname)s %(message)s',
                        datefmt='%d %b %Y %H:%M:%S')
    main()
