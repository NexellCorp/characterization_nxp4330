/*
   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   Balanced binary tree ported from glib by Sven Neumann
   <sven@convergence.de>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __DIRECT__TREE_H__
#define __DIRECT__TREE_H__

#include <direct/types.h>


typedef struct __D_DirectNode DirectNode;

struct __D_DirectTree
{
     DirectNode *root;
     void       *fast_keys[128];
};

struct __D_DirectNode
{
     int         balance;
     DirectNode *left;
     DirectNode *right;
     void       *key;
     void       *value;
};


DirectTree DIRECT_API *direct_tree_new    ( void );

void       DIRECT_API  direct_tree_destroy( DirectTree *tree );

void       DIRECT_API  direct_tree_insert ( DirectTree *tree,
                                            void       *key,
                                            void       *value );

void       DIRECT_API *direct_tree_lookup ( DirectTree *tree,
                                            void       *key );

#endif
