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
        self.catalog_id = -1
        self.tag = []
        self.zan_num = 0
        self.comment_num = 0
        self.desc = ''
        self.score = -1
        self.ctime = 0

def _get_content(imgs):
    content = ''
    try:
        imgs = json.loads(imgs)
        for img in imgs:
            content += img.get('content', '')
    except Exception, e:
        logging.info("get content error, e[%s]", e)

    return content

def _load_tweet(file):
    tweet_dict = {}#tid:dict(info)
    with open(file) as f:
        logging.info("load tweet from [%s]" % (file))
        for index, line in enumerate(f):
            info = {}
            if index == 0:
                continue
            line = line.strip('\n')
            item = line.split('\t')
            tid = item[0]
            info['type'] = item[1]
            info['content'] = _get_content(item[2])
            info['s_catalog'] = item[3]
            info['tags'] = item[4].split(',')
            info['f_catalog'] = item[5]
            info['ctime'] = item[6]

            tweet_dict[tid] = info
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
            tid = item[0]
            uid = item[1]
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
            tid = item[1]
            if tid in comment_dict:
                comment_dict[tid] += 1
            else:
                comment_dict[tid] = 1
    return comment_dict

def _load_catalog_id(file):
    catalog_dict = {} #name:id
    logging.info('load catalog [%s]' % (file))
    
    with open(file) as f:
        for index, line in enumerate(f):
            line = line.strip('\n')
            item = line.split('\t')
            catalog_id = item[0]
            catalog = item[1]
            parent_id = item[2]
            parent_catalog = item[3]
            if parent_id != 0: #是二级分类
                catalog_dict[parent_catalog+catalog] = catalog_id

    return catalog_dict

def _load_tag_dict(file):
    tag_dict = {}
    logging.info('load tag [%s]' % (file))

    with open(file) as f:
        for index, line in enumerate(f):
            line = line.strip('\n')
            item = line.split('\t')
            tag_name = item[0]
            tag_id = item[1]
            tag_dict[tag_name] = tag_id

def _get_score(tweet):
    G = 1.8
    zan_num = int(tweet.zan_num)
    comment_num = int(tweet.comment_num)
    ctime = int(tweet.ctime)

    t = (time.time() - ctime)/3600.0

    score = (0.6*zan_num + 0.4*comment_num)/(t+2)**G

    return score



def merge_data(out_file, tweet_file, zan_file, comment_file, catalog_file, tag_file):
    tweet_dict = _load_tweet(tweet_file)
    zan_dict = _load_zan(zan_file)
    comment_dict = _load_comment(comment_file)
    catalog_dict = _load_catalog_id(catalog_file)
    #tag_dict = _load_tag_id(tag_file)

    logging.info('total tweet: %d' % (len(tweet_dict)))
    #fp_out = codecs.open(out_file, 'w', "utf-8")
    fp_out = open(out_file, 'w')
    tweet_list = []
    for tid in tweet_dict:
        info = tweet_dict[tid]
        tweet = Tweet()
        tweet.tid = tid 
        tweet.type = info['type']

        catalog = info['f_catalog'] + info['s_catalog']
        catalog_id = catalog_dict.get(catalog, -1)
        tweet.catalog_id = catalog_id

        zan_num = zan_dict.get(tid, 0)
        tweet.zan_num = zan_num
        comment_num = comment_dict.get(tid, 0)
        tweet.comment_num = comment_num
        tag = info['tags']

        tweet.tag = tag
        tweet.desc = info['content']
        tweet.ctime = info['ctime']
        if ' '.join(tweet.tag).strip() != '' or tweet.desc.strip() != '':
            tweet.score = _get_score(tweet)
            tweet_list.append(tweet)
    tweet_list = sorted(tweet_list, key=lambda x:x.score)

    for tweet in tweet_list:
        line = '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' % (tweet.tid, tweet.type, tweet.catalog_id, ' '.join(tweet.tag), tweet.zan_num, tweet.comment_num, tweet.desc)
        logging.info('line:%s' % line)
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
    catalog_file = sys.argv[5]
    tag_file = sys.argv[6]
    logging.info('merge_data: output[%s] tweet[%s] zan[%s] comment[%s] catalog[%s] tag[%s]' % (output_file, tweet_file, zan_file, comment_file, catalog_file, tag_file))
    try:
        merge_data(output_file, tweet_file, zan_file, comment_file, catalog_file, tag_file)
    except Exception as e:
        logging.warning(traceback.format_exc())
        exit(1)


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO,
                        format='%(asctime)s %(levelname)s %(message)s',
                        datefmt='%d %b %Y %H:%M:%S')
    main()
