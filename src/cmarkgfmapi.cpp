/***********************************************************************
 *
 * Copyright (C) 2020 wereturtle
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#include "cmark-gfm-core-extensions.h"
#include "cmark-gfm-extension_api.h"

#include <QMutex>

#include "cmarkgfmapi.h"

#include "cmark-gfm-core-extensions.h"
#include "cmark-gfm-extension_api.h"

namespace ghostwriter
{
class CmarkGfmAPIPrivate
{
public:

    static CmarkGfmAPI *instance;

    CmarkGfmAPIPrivate()
    {
        ;
    };

    ~CmarkGfmAPIPrivate()
    {
        ;
    };

    // cmark-gfm extensions to enable.
    cmark_syntax_extension *tableExt;
    cmark_syntax_extension *strikethroughExt;
    cmark_syntax_extension *autolinkExt;
    cmark_syntax_extension *tagfilterExt;
    cmark_syntax_extension *tasklistExt;

    QMutex apiMutex;
};

CmarkGfmAPI *CmarkGfmAPIPrivate::instance = NULL;

CmarkGfmAPI *CmarkGfmAPI::instance()
{
    if (NULL == CmarkGfmAPIPrivate::instance) {
        CmarkGfmAPIPrivate::instance = new CmarkGfmAPI();
    }

    return CmarkGfmAPIPrivate::instance;
}

CmarkGfmAPI::~CmarkGfmAPI()
{
    cmark_arena_reset();
}

MarkdownAST *CmarkGfmAPI::parse(const QString &text, const bool smartTypographyEnabled)
{
    int opts = CMARK_OPT_DEFAULT | CMARK_OPT_FOOTNOTES;

    if (smartTypographyEnabled) {
        opts |= CMARK_OPT_SMART;
    }

    d_func()->apiMutex.lock();

    cmark_mem *mem = cmark_get_arena_mem_allocator();
    cmark_parser *parser = cmark_parser_new_with_mem(opts, mem);

    cmark_parser_attach_syntax_extension(parser, d_func()->tableExt);
    cmark_parser_attach_syntax_extension(parser, d_func()->strikethroughExt);
    cmark_parser_attach_syntax_extension(parser, d_func()->autolinkExt);
    cmark_parser_attach_syntax_extension(parser, d_func()->tagfilterExt);
    cmark_parser_attach_syntax_extension(parser, d_func()->tasklistExt);

    cmark_parser_feed(parser, text.toLatin1().data(), text.length());

    cmark_node *root = cmark_parser_finish(parser);
    MarkdownAST *ast = new MarkdownAST(root);
    cmark_parser_free(parser);
    cmark_node_free(root);
    cmark_arena_reset();

    d_func()->apiMutex.unlock();

    return ast;
}

QString CmarkGfmAPI::renderToHtml(const QString &text, const bool smartTypographyEnabled)
{
    int opts = CMARK_OPT_DEFAULT | CMARK_OPT_FOOTNOTES;

    if (smartTypographyEnabled) {
        opts |= CMARK_OPT_SMART;
    }

    d_func()->apiMutex.lock();

    cmark_mem *mem = cmark_get_arena_mem_allocator();
    cmark_parser *parser = cmark_parser_new_with_mem(opts, mem);

    cmark_parser_attach_syntax_extension(parser, d_func()->tableExt);
    cmark_parser_attach_syntax_extension(parser, d_func()->strikethroughExt);
    cmark_parser_attach_syntax_extension(parser, d_func()->autolinkExt);
    cmark_parser_attach_syntax_extension(parser, d_func()->tagfilterExt);
    cmark_parser_attach_syntax_extension(parser, d_func()->tasklistExt);

    cmark_parser_feed(parser, text.toLatin1().data(), text.length());

    cmark_node *root = cmark_parser_finish(parser);
    char *output = cmark_render_html(root, opts, cmark_parser_get_syntax_extensions(parser));
    QString html = QString::fromUtf8(output);

    cmark_parser_free(parser);
    cmark_arena_reset();

    d_func()->apiMutex.unlock();

    return html;
}

CmarkGfmAPI::CmarkGfmAPI()
    : d_ptr(new CmarkGfmAPIPrivate())
{
    cmark_gfm_core_extensions_ensure_registered();
    d_func()->tableExt = cmark_find_syntax_extension("table");
    d_func()->strikethroughExt = cmark_find_syntax_extension("strikethrough");
    d_func()->autolinkExt = cmark_find_syntax_extension("autolink");
    d_func()->tagfilterExt = cmark_find_syntax_extension("tagfilter");
    d_func()->tasklistExt = cmark_find_syntax_extension("tasklist");
}
}