/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "productCategories.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <parameter.h>
#include <qstatusbar.h>
#include <openreports.h>
#include "productCategory.h"

/*
 *  Constructs a productCategories as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
productCategories::productCategories(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_prodcat, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_deleteUnused, SIGNAL(clicked()), this, SLOT(sDeleteUnused()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_prodcat, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
productCategories::~productCategories()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void productCategories::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <Q3PopupMenu>


void productCategories::init()
{
  statusBar()->hide();
  
  _prodcat->addColumn(tr("Category"),    70, Qt::AlignLeft );
  _prodcat->addColumn(tr("Description"), -1, Qt::AlignLeft );

  if (_privleges->check("MaintainProductCategories"))
  {
    connect(_prodcat, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_prodcat, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_prodcat, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_prodcat, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

    _new->setEnabled(FALSE);
    _deleteUnused->setEnabled(FALSE);
  }

  sFillList(-1);
}

void productCategories::sDelete()
{
  q.prepare("SELECT deleteProductCategory(:prodcat_id) AS result;");
  q.bindValue(":prodcat_id", _prodcat->id());
  q.exec();
  if (q.first())
  {
    switch (q.value("result").toInt())
    {
      case -1:
        QMessageBox::warning( this, tr("Cannot Delete Product Category"),
                              tr( "You cannot delete the selected Product Category because there are currently items assigned to it.\n"
                                  "You must first re-assign these items before deleting the selected Product Category." ) );
        return;
    }

    sFillList(-1);
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}

void productCategories::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  productCategory newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != QDialog::Rejected)
    sFillList(result);
}

void productCategories::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("prodcat_id", _prodcat->id());

  productCategory newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != QDialog::Rejected)
    sFillList(result);
}

void productCategories::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prodcat_id", _prodcat->id());

  productCategory newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void productCategories::sPopulateMenu( Q3PopupMenu * menu )
{
  int menuItem;

  menuItem = menu->insertItem("Edit Product Cateogry...", this, SLOT(sEdit()), 0);
  if (!_privleges->check("MaintainProductCategories"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem("Delete Product Category...", this, SLOT(sDelete()), 0);
  if (!_privleges->check("MaintainProductCategories"))
    menu->setItemEnabled(menuItem, FALSE);
}

void productCategories::sPrint()
{
  orReport report("ProductCategoriesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void productCategories::sDeleteUnused()
{
  if ( QMessageBox::warning( this, tr("Delete Unused Product Categories"),
                             tr("Are you sure that you wish to delete all unused Product Categories?"),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0 )
  {
    q.exec("SELECT deleteUnusedProductCategories() AS result;");
    sFillList(-1);
  }
}

void productCategories::sFillList(int pId)
{
  _prodcat->populate( "SELECT prodcat_id, prodcat_code, prodcat_descrip "
                      "FROM prodcat "
                      "ORDER BY prodcat_code;", pId );
}

