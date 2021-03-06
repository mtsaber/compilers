// Graham Greving
// ggreving@ucsc.edu
// CMPS104a: asg4: astree.cpp

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"
#include "typecheck.h"

astree* new_astree (int symbol, int filenr, int linenr, int offset,
                    const char* lexinfo) {
   astree* tree = new astree();
   tree->symbol = symbol;
   tree->filenr = filenr;
   tree->linenr = linenr;
   tree->offset = offset;
   tree->lexinfo = intern_stringset (lexinfo);
   tree->attributes = 0;
   tree->block_nr = 0;
   tree->struct_entry = NULL;
   DEBUGF ('f', "astree %p->{%d:%d.%d: %s: \"%s\"}\n",
           tree, tree->filenr, tree->linenr, tree->offset,
           get_yytname (tree->symbol), tree->lexinfo->c_str());
   return tree;
}

astree* change_sym(astree *root, int symbol) {
   root->symbol = symbol;
   return root;
}

astree* adopt1 (astree* root, astree* child) {
   root->children.push_back (child);
   DEBUGF ('a', "%p (%s) adopting %p (%s)\n",
           root, root->lexinfo->c_str(),
           child, child->lexinfo->c_str());
   return root;
}

astree* adopt2 (astree* root, astree* left, astree* right) {
   adopt1 (root, left);
   adopt1 (root, right);
   return root;
}

astree* adopt3 (astree* root, astree* left,
       astree* middle, astree* right) {
   adopt1 (root, left);
   adopt1 (root, middle);
   adopt1 (root, right);
   return root;
}

astree* adopt2sym (astree* root, astree* left,
        astree* right, int symbol) {
   root = adopt2(root, left, right);
   root->symbol = symbol;
   return root;
}

astree* adopt1sym (astree* root, astree* child, int symbol) {
   root = adopt1 (root, child);
   root->symbol = symbol;
   return root;
}

// used for printing to the .ast file
// not (normally) called externally -- only by dump_astree_rec()
static void dump_node (FILE* outfile, astree* node) {
   char* tname = (char*) get_yytname(node->symbol);
   if (strstr(tname, "TOK_") == tname) tname += 4;
   fprintf(outfile, "%s \"%s\" (%zu.%zu.%zu) {%lu} %s ",
      tname,
      (node->lexinfo)->c_str(), node->filenr,
      node->linenr, node->offset, node->block_nr,
      get_attr_string(node->attributes));
   if (node->symbol == TOK_STRUCT) {
      fprintf(outfile, "\"%s\"", node->struct_entry->first->c_str());
   }
   if (node->symbol == TOK_IDENT) {
      fprintf(outfile, "(%u.%u.%u)", 0, 0, 0);
   }
   fprintf(outfile, "\n");
}

static void dump_astree_rec (FILE* outfile, astree* root, int depth) {
      if (root == NULL) return;
      for (int i = 0; i < depth; i++) fprintf(outfile, "|\t");
      dump_node (outfile, root);
      for (size_t child = 0; child < root->children.size(); ++child) {
         dump_astree_rec (outfile, root->children[child], depth + 1);
      }
}

// Used for printing to the .tok file (called externally)
void dump_tok (FILE* outfile, astree* node) {
   fprintf(outfile, "%3ld %ld.%03ld %-3d %-15s (%s)\n",
            node->filenr, node->linenr, node->offset,
            node->symbol, get_yytname(node->symbol),
            node->lexinfo->c_str());
}

void dump_astree (FILE* outfile, astree* root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}

void yyprint (FILE* outfile, unsigned short toknum, astree* yyvaluep) {
   DEBUGF ('f', "toknum = %d, yyvaluep = %p\n", toknum, yyvaluep);
   if (is_defined_token (toknum)) {
      dump_node (outfile, yyvaluep);
   }else {
      fprintf (outfile, "%s(%d)\n", get_yytname (toknum), toknum);
   }
   fflush (NULL);
}

void free_ast (astree* root) {
   while (not root->children.empty()) {
      astree* child = root->children.back();
      root->children.pop_back();
      free_ast (child);
   }
   DEBUGF ('f', "free [%X]-> %d:%d.%d: %s: \"%s\")\n",
           (uintptr_t) root, root->filenr, root->linenr, root->offset,
           get_yytname (root->symbol), root->lexinfo->c_str());
   delete root;
}

void free_ast2 (astree* tree1, astree* tree2) {
   free_ast (tree1);
   free_ast (tree2);
}

void free_ast3 (astree* tree1, astree* tree2, astree* tree3) {
   free_ast(tree1);
   free_ast(tree2);
   free_ast(tree3);
}

