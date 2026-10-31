#ifndef BROWSER_ENTRIES_H
#define BROWSER_ENTRIES_H

#include "browser.h"

class BrowserFile : public Entry{

    public:
        bool selected;
        string fileSize;
        int filetype;
        string parent;

    protected:
        virtual unsigned getFileSize();

    public:
    
        BrowserFile();
    
        BrowserFile(string path);
        BrowserFile(string parent, string name);
        
        ~BrowserFile();
        
        void calcSize();
        
        void changeSelection();
        
        char* getType();
        
        char* getSubtype();
        
        void loadIcon();

        void freeIcon();
        
        void loadPics();
        
        void loadAVMedia();
        
        void doExecute();

};

class BrowserFolder : public BrowserFile{
    public:
        BrowserFolder(string path);
        BrowserFolder(string parent, string name);
        
        ~BrowserFolder();

        string getName();
        
        char* getType();
        
        char* getSubtype();

        void loadIcon();
};

#endif
