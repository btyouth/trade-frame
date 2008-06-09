#pragma once

#include <string>

#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;
using namespace boost::gregorian;

#include "BarFactory.h"
#include "TimeSeries.h"
#include "ProviderInterface.h"
#include "Instrument.h"
#include "Order.h"

class CBasketTradeSymbolInfo {
public:
  explicit CBasketTradeSymbolInfo( 
    const std::string &sSymbolName, const std::string &sPath, const std::string &sStrategy,
    CProviderInterface *pExecutionProvider );
  ~CBasketTradeSymbolInfo( void );

  void CalculateTrade( ptime dtTradeDate, double dblFunds );
  double GetProposedEntryCost() { return m_dblProposedEntryCost; };
  int GetQuantityForEntry() { return m_nQuantityForEntry; };
  void HandleTrade( const CTrade &trade );
  void HandleOpen( const CTrade &trade );
  const std::string &GetSymbolName( void ) { return m_sSymbolName; };
protected:
  std::string m_sSymbolName;
  std::string m_sPath;
  std::string m_sStrategy;
  CInstrument *m_pInstrument;
  ptime m_dtTradeDate;
  double m_dblMaxAllowedFunds;
  double m_dblDayOpenPrice;
  double m_dblPriceForEntry;
  double m_dblAveragePriceOfEntry;
  double m_dblMarketValueAtEntry;
  double m_dblCurrentMarketPrice;
  double m_dblCurrentMarketValue;
  int m_nQuantityForEntry;
  int m_nWorkingQuantity;
  double m_dblAllocatedWorkingFunds;
  double m_dblExitPrice;
  double m_dblProposedEntryCost;
  //bool m_bEntryInitiated;
  enum enumPositionState { Init, WaitingForOpen, 
    WaitingForThe3Bars, 
    WaitingForOrderFulfillmentLong, WaitingForOrderFulfillmentShort,
    WaitingForLongExit, WaitingForShortExit,
    WaitingForExit, Exited } m_PositionState;
  ptime m_dtToday;
  bool m_bOpenFound;
  double m_dblOpen;
  double m_dblStop;

  CProviderInterface *m_pExecutionProvider;

  CBarFactory m_1MinBarFactory;
  CBars m_bars;
  void HandleBarFactoryBar( const CBar &bar );

  void HandleOrderFilled( COrder *pOrder );
  bool m_bDoneTheLong, m_bDoneTheShort;

private:
  CBasketTradeSymbolInfo( const CBasketTradeSymbolInfo & );  // disallow copy construction
};