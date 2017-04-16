#!/usr/bin/python3
# -*- coding: utf8 -*-

import glob
import json
import os
import shutil
import urllib
import zipfile

from queue import Queue
from threading import Thread
from urllib import request

#-----------------------------------------------------------------------------------------
# from thread_pipeline.py

class ClosableQueue(Queue) :
    SENTINEL = object()

    def close(self) :
        self.put(self.SENTINEL)

    def __iter__(self) :
        while True :
            item = self.get()

            try :
                if item is self.SENTINEL :
                    return
                yield item
            finally :
                self.task_done()

class StoppableWorker(Thread) :
    def __init__(self, func, in_queue, out_queue) :
        super().__init__()
        self.func = func
        self.in_queue = in_queue
        self.out_queue = out_queue
        self.polled_count = 0
        self.work_done = 0

    def run(self):
        for item in self.in_queue :
            result = self.func(item)
            if result is not None :
                self.out_queue.put(result)

#-----------------------------------------------------------------------------------------
# downalod

def DownloadFile(item) :
    url = item['url']
    filename = item['filename']
    print('download :', url)

    with urllib.request.urlopen(url) as response :
        with open(filename, "wb") as outfile :
            print(filename)
            shutil.copyfileobj(response, outfile)
            return filename

    return filename

def DecompressZip(filename) :
    if filename :
        try :
            print('extract : ', filename)
            absdir = os.path.dirname(os.path.abspath(filename))
            zfile = zipfile.ZipFile(filename, 'r')
            zfile.extractall(absdir)
            return filename
        except :
            print('extractfail %s' % filename)

    return filename

#-----------------------------------------------------------------------------------------
# main

def CanSkipDownload(path, metas) :
    for obj in glob.glob(path + '*') :
        obj = os.path.basename(obj)
        for meta in metas :      
            if obj.lower().startswith(meta.lower()) :
                return True

    return False

if __name__ == '__main__' :

    jsonfile = './_ExternlLibrary.json'
    with open(jsonfile, 'rt') as file :
        externalLibrary = json.load(file)
    
    download_queue = ClosableQueue()
    unzip_queue = ClosableQueue()
    done_queue = ClosableQueue()    
    queues = [download_queue, unzip_queue]

    externlLibrary = './_ExternlLibrary/'
    if not os.path.exists(externlLibrary) :
        os.makedirs(externlLibrary)
        print('mkdir ', externlLibrary)
    
    threads = [
        StoppableWorker(DownloadFile, download_queue, unzip_queue),
        StoppableWorker(DecompressZip, unzip_queue, done_queue),
    ]

    for thread in threads : 
        thread.start()

    for downinfo in externalLibrary['downloads'] :
        if CanSkipDownload(externlLibrary, downinfo['meta']) == False:
            downloaditem = {'url' : downinfo['url'], 'filename' : externlLibrary + downinfo['filename']}
            download_queue.put(downloaditem)
        else :
            print('skip download ', downinfo['filename'])

    for queue in queues :        
        queue.close()
        queue.join()

    print('done ', done_queue.qsize())
