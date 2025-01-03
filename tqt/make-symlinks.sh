#!/bin/sh
cd include
rm -f q*.h
ln -s ../src/*/q*.h .
ln -s ../extensions/*/src/q*.h .
ln -s ../tools/assistant/lib/ntqassistantclient.h .
ln -s ../tools/designer/uilib/ntqwidgetfactory.h .
rm -f q*_p.h
cd private
rm -f q*_p.h
ln -s ../../src/*/q*_p.h .

