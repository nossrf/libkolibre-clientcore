/*
 * Copyright (C) 2012 Kolibre
 *
 * This file is part of kolibre-clientcore.
 *
 * Kolibre-clientcore is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Kolibre-clientcore is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with kolibre-clientcore. If not, see <http://www.gnu.org/licenses/>.
 */

#include "DaisyOnlineBookNode.h"
#include "Commands/InternalCommands.h"
#include "CommandQueue2/CommandQueue.h"
#include "DaisyNavi.h"
#include "Defines.h"

#include <NaviEngine.h>
#include <DaisyOnlineHandler.h>

#include <cstring>
#include <algorithm>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.clientcore
log4cxx::LoggerPtr bookNodeLog(log4cxx::Logger::getLogger("kolibre.clientcore.daisyonlinebooknode"));

using namespace naviengine;

DaisyOnlineBookNode::DaisyOnlineBookNode(std::string book_id, DaisyOnlineHandler *DOHandler) :
        book_id_(book_id), pDOHandler(DOHandler), daisyNaviActive(false)
{
    LOG4CXX_DEBUG(bookNodeLog, "constructor");
    play_before_onOpen_ = "";
    lastError = (errorType) -1;
    pDaisyNavi = new DaisyNavi;
}

DaisyOnlineBookNode::~DaisyOnlineBookNode()
{
    LOG4CXX_DEBUG(bookNodeLog, "destructor");
    delete pDaisyNavi;
}

bool DaisyOnlineBookNode::up(NaviEngine& navi)
{
    if (not pDaisyNavi->up(navi))
    {
        // DaisyNavi has moved beyond TOPLEVEL
        daisyNaviActive = false;
        if (pDaisyNavi->isOpen())
        {
            pDaisyNavi->closeBook();
        }

        // DaisyNavi fiddled with the state so we reset it.
        navi.setCurrentNode(this);

        // Move up one more level since the book node doesn't do
        // anything other than setup daisynavi.
        return VirtualMenuNode::up(navi);
    }

    return true;
}

bool DaisyOnlineBookNode::prev(NaviEngine& navi)
{
    return pDaisyNavi->prev(navi);
}

bool DaisyOnlineBookNode::next(NaviEngine& navi)
{
    return pDaisyNavi->next(navi);
}

bool DaisyOnlineBookNode::select(NaviEngine& navi)
{
    return pDaisyNavi->select(navi);
}

bool DaisyOnlineBookNode::selectByUri(naviengine::NaviEngine& navi, std::string uri)
{
    return pDaisyNavi->selectByUri(navi, uri);
}

bool DaisyOnlineBookNode::menu(NaviEngine& navi)
{
    return pDaisyNavi->menu(navi);
}

bool DaisyOnlineBookNode::onOpen(NaviEngine& navi)
{
    if (daisyNaviActive)
    {
        return pDaisyNavi->onOpen(navi);
    }

    time_t starttime, timenow;
    time(&starttime);

    Narrator::Instance()->play(_N("opening publication"));

    url_ = "";
    pDaisyNavi->parent_ = this;

    // invoke getContentResouces
    kdo::ContentResources *contentResources = pDOHandler->getContentResources(book_id_);

    if (!pDOHandler->good())
    {
        lastError = DO_INVOKE_ERROR;
        LOG4CXX_ERROR(bookNodeLog, "getContentResources failed for contentID " << book_id_);
        Narrator::Instance()->play(_N("error loading data"));
        // wait for narrator to finish speaking
        while (Narrator::Instance()->isSpeaking())
        {
            usleep(20000);
        };
        //ErrorMessage error(NETWORK, pDOHandler->getErrorMessage());
        //cq2::Command<ErrorMessage> message(error);
        //error();
        cq2::Command<INTERNAL_COMMAND> c(COMMAND_HOME);
        c();
        daisyNaviActive = false;
        delete contentResources;
        return daisyNaviActive;
    }

    if (contentResources == NULL)
    {
        lastError = CONTENTRESOURCES_NULL_ERROR;
        LOG4CXX_ERROR(bookNodeLog, "getContentResources is NULL for contentID " << book_id_);
        Narrator::Instance()->play(_N("error loading data"));
        // wait for narrator to finish speaking
        while (Narrator::Instance()->isSpeaking())
        {
            usleep(20000);
        };
        //ErrorMessage error(SERVICE, pDOHandler->getErrorMessage());
        //cq2::Command<ErrorMessage> message(error);
        //error();
        cq2::Command<INTERNAL_COMMAND> c(COMMAND_HOME);
        c();
        daisyNaviActive = false;
        delete contentResources;
        return daisyNaviActive;
    }

    // loop through each resource and try locating ncc.html

    std::vector<kdo::Resource> resources = contentResources->getResouces();
    int resource_count = resources.size();
    for (int i = 0; i < resource_count; i++)
    {
        time(&timenow);
        // when 3 seconds has passed and more than 1 item is left, play wait jingle
        if ((i == resource_count - 1) && (timenow - starttime > 3))
        {
            Narrator::Instance()->playWait();
        }

        // is this ncc.html file
        std::string filename(resources[i].getLocalUri());
        //filename = tolower(filename.c_str());
        std::transform(filename.begin(), filename.end(), filename.begin(), (int (*)(int))tolower);if
(        filename == "ncc.html" )
        {
            url_ = std::string(resources[i].getUri());
        }

        if (url_ != "")
        {
            LOG4CXX_INFO(bookNodeLog, "Found url to ncc: " << url_);
            break;
        }
    }

    delete contentResources;

    if (url_ == "")
    {
        lastError = NAVIGATION_CONTROL_FILE_ERROR;
        LOG4CXX_ERROR(bookNodeLog, "url to ncc was not found in resources for contentID " << book_id_);
        Narrator::Instance()->play(_N("content error"));
        // wait for narrator to finish speaking
        while (Narrator::Instance()->isSpeaking())
        {
            usleep(20000);
        };
        //ErrorMessage error(CONTENT, "ncc.html not found in content resources");
        //cq2::Command<ErrorMessage> message(error);
        //error();
        cq2::Command<INTERNAL_COMMAND> c(COMMAND_HOME);
        c();
        daisyNaviActive = false;
        return daisyNaviActive;
    }

    if (pDaisyNavi->open(url_) && pDaisyNavi->onOpen(navi))
    {
        LOG4CXX_INFO(bookNodeLog, "opening contentID " << book_id_);
        daisyNaviActive = true;
        return daisyNaviActive;
    }

    lastError = OPEN_CONTENT_ERROR;
    LOG4CXX_ERROR(bookNodeLog, "not able to open contentID " << book_id_);
    Narrator::Instance()->play(_N("content error"));
    // wait for narrator to finish speaking
    while (Narrator::Instance()->isSpeaking())
    {
        usleep(20000);
    };
    //ErrorMessage error(CONTENT, "not able to open publication");
    //cq2::Command<ErrorMessage> message(error);
    //error();
    cq2::Command<INTERNAL_COMMAND> c(COMMAND_HOME);
    c();

    daisyNaviActive = false;
    return daisyNaviActive;
}

bool DaisyOnlineBookNode::process(NaviEngine& navi, int command, void* data)
{
    bool result = pDaisyNavi->process(navi, command, data);

    // If the book is no longer opened then swap node to parent
    if (daisyNaviActive && !pDaisyNavi->isOpen())
    {
        LOG4CXX_INFO(bookNodeLog, "Closing at the end of book");
        daisyNaviActive = false;
        return VirtualMenuNode::up(navi);
    }

    return result;
}

bool DaisyOnlineBookNode::onNarrate()
{
    // Parent node does the narration
    return true;
}

DaisyOnlineBookNode::errorType DaisyOnlineBookNode::getLastError()
{
    return lastError;
}
