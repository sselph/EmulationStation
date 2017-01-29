#include "scrapers/SselphScraper.h"
#include "HttpReq.h"
#include "Log.h"
#include "pugixml/pugixml.hpp"
#include "PlatformId.h"
#include "Settings.h"
#include "Util.h"

#include <string>
#include <stdlib.h>

void sselph_generate_gdb_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results)
{
	const std::vector<PlatformIds::PlatformId> p = params.system->getPlatformIds();
	std::string pn;
	for(auto it = p.begin(); it != p.end(); it++) {
		pn += getPlatformName(*it);
        	if ( it + 1 != p.end() ) {
            		pn += ",";
        	}
	}
	GoInt32 id = SSelphReq(const_cast<char*>(params.game->getPath().c_str()), const_cast<char*>(pn.c_str()), const_cast<char*>(""));
	requests.push(std::unique_ptr<ScraperRequest>(new ScraperSSelphRequest(results, id)));
}

void sselph_generate_mamedb_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results)
{
	const std::vector<PlatformIds::PlatformId> p = params.system->getPlatformIds();
	std::string pn;
	for(auto it = p.begin(); it != p.end(); it++) {
		pn += getPlatformName(*it);
        	if ( it + 1 != p.end() ) {
            		pn += ",";
        	}
	}
	GoInt32 id = SSelphReq(const_cast<char*>(params.game->getPath().c_str()), const_cast<char*>(pn.c_str()), const_cast<char*>(""));
	requests.push(std::unique_ptr<ScraperRequest>(new ScraperSSelphRequest(results, id)));
}

void sselph_generate_ss_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results)
{
	const std::vector<PlatformIds::PlatformId> p = params.system->getPlatformIds();
	std::string pn;
	for(auto it = p.begin(); it != p.end(); it++) {
		pn += getPlatformName(*it);
        	if ( it + 1 != p.end() ) {
            		pn += ",";
        	}
	}
	std::string imgPath = getSaveAsPath(params, "image", ".png");
  	GoInt32 id = SSelphReq(const_cast<char*>(params.game->getPath().c_str()), const_cast<char*>(pn.c_str()), const_cast<char*>(imgPath.c_str()));
	requests.push(std::unique_ptr<ScraperRequest>(new ScraperSSelphRequest(results, id)));
}

// ScraperSSelphRequest
ScraperSSelphRequest::ScraperSSelphRequest(std::vector<ScraperSearchResult>& resultsWrite, GoInt32 rid) 
	: ScraperRequest(resultsWrite)
{
	setStatus(ASYNC_IN_PROGRESS);
	id = rid;
}

void ScraperSSelphRequest::update()
{
	SSelphResp_return ret = SSelphResp(id);
	if(ret.r1 == 0)
	{
		return;
	}

	setStatus(ASYNC_DONE); // if process() has an error, status will be changed to ASYNC_ERROR
	process(ret.r0, mResults);
	return;
}

void ScraperSSelphRequest::process(char* resp, std::vector<ScraperSearchResult>& results)
{	
	pugi::xml_document doc;
	pugi::xml_parse_result parseResult = doc.load(resp);
	if(!parseResult)
	{
		std::stringstream ss;
		ss << "GamesDBRequest - Error parsing XML. \n\t" << parseResult.description() << "";
		std::string err = ss.str();
		setError(err);
		LOG(LogError) << err;
		return;
	}

	pugi::xml_node game = doc.child("game");
	while(game && results.size() < MAX_SCRAPER_RESULTS)
	{
		ScraperSearchResult result;
		result.crcMatched = game.child("CRCMatch").text().as_bool();
		result.mdl.set("name", game.child("GameTitle").text().get());
		result.mdl.set("desc", game.child("Overview").text().get());
		boost::posix_time::ptime rd = string_to_ptime(game.child("ReleaseDate").text().get(), "%Y%m%dT%H%M%S");
		result.mdl.setTime("releasedate", rd);
		result.mdl.set("developer", game.child("Developer").text().get());
		result.mdl.set("publisher", game.child("Publisher").text().get());
		result.mdl.set("genre", game.child("Genre").first_child().text().get());
		result.mdl.set("players", game.child("Players").text().get());
		if(Settings::getInstance()->getBool("ScrapeRatings") && game.child("Rating"))
		{
			float ratingVal = (game.child("Rating").text().as_int() / 10.0f);
			std::stringstream ss;
			ss << ratingVal;
			result.mdl.set("rating", ss.str());
		}
	
		std::string image_url = game.child("ImageURL").text().get();
		if(!image_url.empty())
		{
			result.thumbnailUrl = image_url;
			result.imageUrl = image_url;
		}
		std::string image_path = game.child("ImagePath").text().get();
		if(!image_path.empty())
		{
			result.mdl.set("image", image_path);
		}
		results.push_back(result);
		game = game.next_sibling("game");
	}
	free(resp);
}
