#pragma once

#include "scrapers/Scraper.h"
#include "libscraper.h"

void sselph_generate_gdb_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results);

void sselph_generate_ss_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results);

void sselph_generate_mamedb_requests(const ScraperSearchParams& params, std::queue< std::unique_ptr<ScraperRequest> >& requests,
	std::vector<ScraperSearchResult>& results);

// a single SSelph request that needs to be processed to get the results
class ScraperSSelphRequest : public ScraperRequest
{
public:
	ScraperSSelphRequest(std::vector<ScraperSearchResult>& resultsWrite, GoInt32 rid);
	virtual void update() override;

protected:
	void process(char* resp, std::vector<ScraperSearchResult>& results);

private:
	GoInt32 id;
};
