/*
 * Copyright (C) 2014 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#ifndef _IMPORT_IMAGE_DIALOG_HH_
#define _IMPORT_IMAGE_DIALOG_HH_

#include "gazebo/gui/qt.h"
#include "gazebo/util/system.hh"

namespace gazebo
{
  namespace gui
  {
    class EditorView;

    /// \addtogroup gazebo_gui
    /// \{

    class GAZEBO_VISIBLE ImportImageDialog : public QDialog
    {
      Q_OBJECT

      /// \brief Constructor
      /// \param[in] _parent Parent QWidget.
      public: ImportImageDialog(QWidget *_parent = 0);

      /// \brief Destructor
      public: virtual ~ImportImageDialog();

      /// \brief A signal used to set the file label.
      /// \param[in] _string File name string.
      signals: void SetFileName(QString _string);

      private slots: void OnSelectFile();
      private slots: void OnAccept();

      private: QLineEdit *fileLineEdit;
      private: QDoubleSpinBox *resolutionSpin;
      private: QGraphicsView *imageDisplay;
      private: int imageDisplayWidth;
      private: int imageDisplayHeight;

      private: EditorView *view;

      private: bool eventFilter(QObject *obj, QEvent *event);
      private: QPointF measureLineStart;
      private: bool drawingLine;
    };
    /// \}
  }
}

#endif
