object Form3: TForm3
  Left = 0
  Top = 0
  Caption = 'Freq_Generator'
  ClientHeight = 238
  ClientWidth = 318
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -14
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 120
  TextHeight = 17
  object Button1: TButton
    Left = 94
    Top = 195
    Width = 98
    Height = 33
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Set'
    TabOrder = 0
    OnClick = Button1Click
  end
  object GroupBox1: TGroupBox
    Left = 31
    Top = 26
    Width = 242
    Height = 143
    Margins.Left = 4
    Margins.Top = 4
    Margins.Right = 4
    Margins.Bottom = 4
    Caption = 'Frequency'
    TabOrder = 1
    object Label1: TLabel
      Left = 25
      Top = 94
      Width = 97
      Height = 17
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      Caption = 'Output Channel'
    end
    object ComboBox1: TComboBox
      Left = 115
      Top = 42
      Width = 64
      Height = 25
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      DropDownCount = 3
      TabOrder = 0
      Text = 'Hz'
      Items.Strings = (
        'Hz'
        'kHz'
        'MHz')
    end
    object Edit1: TEdit
      Left = 25
      Top = 42
      Width = 70
      Height = 25
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      TabOrder = 1
      Text = '10'
    end
    object ComboBox2: TComboBox
      Left = 132
      Top = 90
      Width = 64
      Height = 25
      Margins.Left = 4
      Margins.Top = 4
      Margins.Right = 4
      Margins.Bottom = 4
      DropDownCount = 3
      TabOrder = 2
      Text = '1'
      Items.Strings = (
        '1'
        '2'
        '3'
        '4'
        '5'
        '6'
        '7'
        '8')
    end
  end
end
