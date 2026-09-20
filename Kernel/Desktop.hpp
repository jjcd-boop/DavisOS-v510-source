#pragma once
#include "Graphics.hpp"
#include "Input.hpp"
#include "WindowManager.hpp"
#include "Pci.hpp"
#include "Xhci.hpp"
namespace Davis::Desktop {
enum Theme:u32{Classic=0,Dark=1,Aurora=2,Nebula=3};
struct State{Theme theme;bool startOpen;u64 frames;WindowManager::State wm;Pci::State pci;Xhci::State xhci;char terminal[256];u32 terminalLen;bool programsOpen;bool diagnosticsVisible;u32 diagnosticsPage;bool systemRailVisible;u64 lastKeyEvents;u64 clickEvents,actions; i32 lastAction; i32 keyboardFocus; i32 shellNotice; u64 shellNoticeDetail; bool gamesMenuOpen; bool clockPopup; bool networkPopup; bool audioPopup; u32 uiScale; u32 requestedWidth,requestedHeight; u64 gameTick; i32 planeX,planeY; i32 bulletX,bulletY; bool bulletActive; i32 enemyX,enemyY; u32 planeScore; u8 chessBoard[64]; i32 chessSelected; bool chessWhite; u8 checkersBoard[64]; i32 checkersSelected; bool checkersRed; u8 solitaireRank[28]; bool solitaireUp[28]; i32 solitaireSelected;};
void Init(State&);void Update(State&,Input::State&,Graphics::Surface&);void Draw(State&,Graphics::Surface&,const Input::State&);
}
