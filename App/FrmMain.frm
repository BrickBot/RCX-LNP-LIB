VERSION 5.00
Begin VB.Form FrmMain 
   Caption         =   "LegOS LNP 4 VB"
   ClientHeight    =   915
   ClientLeft      =   855
   ClientTop       =   1140
   ClientWidth     =   2730
   LinkTopic       =   "Form1"
   ScaleHeight     =   61
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   182
   StartUpPosition =   2  'CenterScreen
   Begin VB.TextBox TxtGetRCXString 
      Height          =   315
      Left            =   900
      TabIndex        =   3
      Top             =   480
      Width           =   1635
   End
   Begin VB.CommandButton Command1 
      Caption         =   "Get"
      Height          =   315
      Left            =   120
      TabIndex        =   2
      Top             =   480
      Width           =   735
   End
   Begin VB.TextBox TxtSendRCXString 
      Height          =   315
      Left            =   900
      TabIndex        =   1
      Text            =   "hello"
      Top             =   120
      Width           =   1635
   End
   Begin VB.CommandButton CmdSendToRCX 
      Caption         =   "Send"
      Height          =   315
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   735
   End
End
Attribute VB_Name = "FrmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private Declare Function SendToRCX Lib "RCXLIB.dll" (ByVal RCXSString As String) As Long
Private Declare Function GetFromRCX Lib "RCXLIB.dll" () As String

Private Sub CmdSendToRCX_Click()
SendToRCX TxtSendRCXString.Text
End Sub

Private Sub Command1_Click()
TxtGetRCXString.Text = GetFromRCX()
End Sub

