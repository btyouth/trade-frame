/************************************************************************
 * Copyright(c) 2017, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/
/* 
 * File:   NotebookOptionChains.cpp
 * Author: raymond@burkholder.net
 * 
 * Created on July 2, 2017, 8:16 PM
 */

#include <algorithm>

#include <boost/lexical_cast.hpp>

#include "NotebookOptionChains.h"

namespace ou { // One Unified
namespace tf { // TradeFrame

NotebookOptionChains::NotebookOptionChains(): wxNotebook() {
  Init();
}

NotebookOptionChains::NotebookOptionChains( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name )
: wxNotebook()
{
  Init();
  Create(parent, id, pos, size, style, name );
}

NotebookOptionChains::~NotebookOptionChains() {
}

void NotebookOptionChains::Init() {
  m_bBound = false;
}

bool NotebookOptionChains::Create( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) {
  
  Init();
  
  wxNotebook::Create(parent, id, pos, size, style, name );
  
  CreateControls();

  return true;
}

void NotebookOptionChains::BindEvents() {
  if ( !m_bBound ) {
    // Page Change events cause issues during OnDestroy
    Bind( wxEVT_NOTEBOOK_PAGE_CHANGING, &NotebookOptionChains::OnPageChanging, this );
    Bind( wxEVT_NOTEBOOK_PAGE_CHANGED, &NotebookOptionChains::OnPageChanged, this );

    //Bind( wxEVT_PAINT, &WinChartView::HandlePaint, this );
    //Bind( wxEVT_SIZE, &GridOptionDetails::HandleSize, this );

    //Bind( wxEVT_MOTION, &WinChartView::HandleMouse, this );
    //Bind( wxEVT_MOUSEWHEEL, &WinChartView::HandleMouseWheel, this );
    //Bind( wxEVT_ENTER_WINDOW, &WinChartView::HandleMouseEnter, this );  
    //Bind( wxEVT_LEAVE_WINDOW, &WinChartView::HandleMouseLeave, this );

    m_bBound = true;
  }
}

void NotebookOptionChains::CreateControls() {   
  
  //Bind( wxEVT_CLOSE_WINDOW, &WinChartView::OnClose, this );  // not called for child windows
  Bind( wxEVT_DESTROY, &NotebookOptionChains::OnDestroy, this );

  BindEvents();
  
}

// start leaving old page
void NotebookOptionChains::OnPageChanging( wxBookCtrlEvent& event ) {
  int ixTab = event.GetOldSelection();
  if ( -1 != ixTab ) {
    //std::cout << "page changing: " << ixTab << std::endl;
    mapOptionExpiry_t::iterator iter 
     = std::find_if( m_mapOptionExpiry.begin(), m_mapOptionExpiry.end(), [ixTab,this](mapOptionExpiry_t::value_type& vt) {
       return ixTab == vt.second.ixTab;
    });
    if ( m_mapOptionExpiry.end() == iter ) {
      std::cout << "NotebookOptionChains::OnPageChanging: couldn't find tab index: " << ixTab << std::endl;
    }
    else {
      iter->second.pWinOptionChain->TimerDeactivate();
      if ( nullptr != m_fOnPageChanging ) {
        m_fOnPageChanging( iter->first );
      }
    }
  }
  event.Skip();
}

// finishing arriving at new page
void NotebookOptionChains::OnPageChanged( wxBookCtrlEvent& event ) {
  int ixTab = event.GetSelection();
  //std::cout << "page changed: " << ixTab << std::endl;
  mapOptionExpiry_t::iterator iter 
   = std::find_if( m_mapOptionExpiry.begin(), m_mapOptionExpiry.end(), [ixTab,this](mapOptionExpiry_t::value_type& vt) {
     return ixTab == vt.second.ixTab;
  });
  if ( m_mapOptionExpiry.end() == iter ) {
    std::cout << "NotebookOptionChains::OnPageChanged: couldn't find tab index: " << ixTab << std::endl;
  }
  else {
    iter->second.pWinOptionChain->TimerActivate();
    if ( nullptr != m_fOnPageChanged ) {
      m_fOnPageChanged( iter->first );
    }
  }
  event.Skip();
}

void NotebookOptionChains::SetName( const std::string& sName ) {
  wxNotebook::SetName( sName );
  m_sName = sName;
}

void NotebookOptionChains::Add( boost::gregorian::date date, double strike, ou::tf::OptionSide::enumOptionSide side, const std::string& sSymbol ) {
  
  mapOptionExpiry_t::iterator iterExpiry = m_mapOptionExpiry.find( date );
  
  if ( m_mapOptionExpiry.end() == iterExpiry ) {
    // add another panel
    std::string sDate = boost::lexical_cast<std::string>( date.year() );
    sDate += std::string( "/" ) 
      + ( date.month().as_number() < 10 ? "0" : "" ) 
      + boost::lexical_cast<std::string>( date.month().as_number() );
    sDate += std::string( "/" ) + ( date.day()   < 10 ? "0" : "" ) + boost::lexical_cast<std::string>( date.day() );
    
    auto* pPanel = new wxPanel( this, wxID_ANY );
    auto* pSizer = new wxBoxSizer(wxVERTICAL);
    pPanel->SetSizer( pSizer );
    auto* pDetails = new GridOptionChain( pPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, sSymbol );
    pSizer->Add( pDetails, 1, wxALL|wxEXPAND, 1 );
    pDetails->m_fOnRowClicked = [this, date](double strike, const GridOptionChain::OptionUpdateFunctions& funcsCall, const GridOptionChain::OptionUpdateFunctions& funcsPut  ){ 
      if ( nullptr != m_fOnRowClicked) 
        m_fOnRowClicked( date, strike, funcsCall, funcsPut );
    };
    
    iterExpiry = m_mapOptionExpiry.insert( 
      m_mapOptionExpiry.begin(), mapOptionExpiry_t::value_type( date, Tab( sDate, pDetails ) ) );
    
    struct Reindex {
      size_t ix;
      Reindex(): ix{} {}
      void operator()( Tab& tab ) { tab.ixTab = ix; ix++; }
    };
    
    // renumber the pages
    Reindex reindex; 
    std::for_each( 
      m_mapOptionExpiry.begin(), m_mapOptionExpiry.end(), 
        [&reindex](mapOptionExpiry_t::value_type& v){ reindex( v.second ); } );
        
    InsertPage( iterExpiry->second.ixTab, pPanel, sDate );
    
    SetSelection( 0 );
  } // end add panel
  
  mapStrike_t& mapStrike( iterExpiry->second.mapStrike ); // assumes single thread
  mapStrike_t::iterator iterStrike = mapStrike.find( strike );
  
  if ( mapStrike.end() == iterStrike ) {
    iterStrike = mapStrike.insert( mapStrike.begin(), mapStrike_t::value_type( strike, Row( mapStrike.size() ) ) );
  }
  
  switch ( side ) {
    case ou::tf::OptionSide::Call:
      assert( "" == iterStrike->second.sCall );
      iterStrike->second.sCall = sSymbol;
      break;
    case ou::tf::OptionSide::Put:
      assert( "" == iterStrike->second.sPut );
      iterStrike->second.sPut = sSymbol;
      break;
  }
  
  // add option set to the expiry panel
  iterExpiry->second.pWinOptionChain->Add( strike, side, sSymbol );
  
}

void NotebookOptionChains::UnbindEvents() {
  if ( m_bBound ) {
    // Page change events occur during Deletion of Pages, causing problems
    assert( Unbind( wxEVT_NOTEBOOK_PAGE_CHANGING, &NotebookOptionChains::OnPageChanging, this ) );
    assert( Unbind( wxEVT_NOTEBOOK_PAGE_CHANGED, &NotebookOptionChains::OnPageChanged, this ) );

    //Unbind( wxEVT_PAINT, &WinChartView::HandlePaint, this );
    //Unbind( wxEVT_SIZE, &GridOptionDetails::HandleSize, this );

    //Unbind( wxEVT_MOTION, &WinChartView::HandleMouse, this );
    //Unbind( wxEVT_MOUSEWHEEL, &WinChartView::HandleMouseWheel, this );
    //Unbind( wxEVT_ENTER_WINDOW, &WinChartView::HandleMouseEnter, this );  
    //Unbind( wxEVT_LEAVE_WINDOW, &WinChartView::HandleMouseLeave, this );
    
    m_bBound = false;
  }
}

void NotebookOptionChains::OnDestroy( wxWindowDestroyEvent& event ) {

  UnbindEvents();

  DeleteAllPages();

  assert( Unbind( wxEVT_DESTROY, &NotebookOptionChains::OnDestroy, this ) );
  
  event.Skip();  // auto followed by Destroy();
}

void NotebookOptionChains::Save( boost::archive::text_oarchive& oa) {
  //oa & m_splitter->GetSashPosition();
  //m_pTreeOps->Save<TreeItemRoot>( oa );
}

void NotebookOptionChains::Load( boost::archive::text_iarchive& ia) {
  //int pos;
  //ia & pos;
  //m_splitter->SetSashPosition( pos );
  //m_pTreeOps->Load<TreeItemRoot>( ia );
}
wxBitmap NotebookOptionChains::GetBitmapResource( const wxString& name ) {
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon NotebookOptionChains::GetIconResource( const wxString& name ) {
    wxUnusedVar(name);
    return wxNullIcon;
}

} // namespace tf
} // namespace ou
