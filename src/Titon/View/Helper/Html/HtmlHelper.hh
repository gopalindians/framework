<?hh // strict
/**
 * @copyright   2010-2014, The Titon Project
 * @license     http://opensource.org/licenses/bsd-license.php
 * @link        http://titon.io
 */

namespace Titon\View\Helper\Html;

use Titon\View\Helper\AbstractHelper;
use Titon\Utility\Config;
use Titon\Utility\Crypt;
use Titon\Utility\Traverse;

/**
 * The HtmlHelper is primarily used for dynamic HTML tag creation within templates.
 *
 * @package Titon\View\Helper\Html
 */
class HtmlHelper extends AbstractHelper {

    /**
     * Mapping of HTML tags for this helper.
     *
     * @type tags
     */
    protected tags $_tags = Map {
        'anchor'    => '<a{attr}>{body}</a>',
        'link'      => '<link{attr}>',
        'meta'      => '<meta{attr}>',
        'script'    => '<script{attr}>{body}</script>',
        'style'     => '<style{attr}>{body}</style>',
        'image'     => '<img{attr}>'
    };

    /**
     * Create an HTML anchor link.
     *
     * @param string $title
     * @param string $url
     * @param attributes $attributes
     * @return string
     */
    public function anchor(string $title, string $url, attributes $attributes = Map {}): string {
        $attributes['href'] = $url;

        return $this->tag('anchor', Map {
            'attr' => $this->attributes($attributes),
            'body' => $this->escape($title)
        });
    }

    /**
     * Return the HTML5 doctype.
     *
     * @return string
     */
    public function doctype(): string {
        return '<!DOCTYPE html>' . PHP_EOL;
    }

    /**
     * Create an image element.
     *
     * @param string $path
     * @param attributes $attributes
     * @param mixed $url
     * @return string
     */
    public function image(string $path, attributes $attributes = Map {}, mixed $url = ''): string {
        if (!isset($attributes['alt'])) {
            $attributes['alt'] = '';
        }

        $attributes['src'] = $path;

        $image = $this->tag('image', Map {
            'attr' => $this->attributes($attributes)
        });

        if ($url) {
            return $this->tag('anchor', Map {
                'attr' => $this->attributes(Map {'href' => $url}),
                'body' => trim($image)
            });
        }

        return $image;
    }

    /**
     * Create a link element.
     *
     * @param string $path
     * @param attributes $attributes
     * @return string
     */
    public function link(string $path, attributes $attributes = Map {}) {
        $attributes = Traverse::merge(Map {
            'rel'   => 'stylesheet',
            'type'  => 'text/css',
            'media' => 'screen'
        }, $attributes);

        $attributes['href'] = $path;

        return $this->tag('link', Map {
            'attr' => $this->attributes($attributes)
        });
    }

    /**
     * Creates a mailto hyperlink. Emails will be obfuscated to hide against spambots and harvesters.
     *
     * @param string $email
     * @param attributes $attributes
     * @return string
     */
    public function mailto(string $email, attributes $attributes = Map {}) {
        $email = Crypt::obfuscate($email);

        if (!isset($attributes['title'])) {
            $attributes['title'] = '';
        }

        $attributes['escape'] = ['href'];
        $attributes['href'] = 'mailto:' . $email;

        return $this->tag('anchor', Map {
            'attr' => $this->attributes($attributes),
            'body' => $email
        });
    }

    /**
     * Create a meta element. Has predefined values for common meta tags.
     *
     * @param string|Map $type
     * @param string $content
     * @param attributes $attributes
     * @return string
     */
    public function meta(mixed $type, string $content = '', attributes $attributes = Map {}) {
        if ($type instanceof Map) {
            return $this->tag('meta', Map {
                'attr' => $this->attributes($type)
            });
        } else {
            $type = mb_strtolower((string) $type);
        }

        if (empty($content)) {
            switch ($type) {
                case 'content-script-type':
                    $content = 'text/javascript';
                break;
                case 'content-style-type':
                    $content = 'text/css';
                break;
                case 'content-type':
                    $content = 'text/html; charset=' . Config::encoding();
                break;
            }
        }

        $metaTypes = Map {
            'content-type'          => Map {'http-equiv' => 'Content-Type', 'content' => $content},
            'content-script-type'   => Map {'http-equiv' => 'Content-Script-Type', 'content' => $content},
            'content-style-type'    => Map {'http-equiv' => 'Content-Style-Type', 'content' => $content},
            'content-language'      => Map {'http-equiv' => 'Content-Language', 'content' => $content},
            'keywords'              => Map {'name' => 'keywords', 'content' => $content},
            'description'           => Map {'name' => 'description', 'content' => $content},
            'author'                => Map {'name' => 'author', 'content' => $content},
            'robots'                => Map {'name' => 'robots', 'content' => $content},
            'rss'                   => Map {'type' => 'application/rss+xml', 'rel' => 'alternate', 'title' => '', 'link' => $content},
            'atom'                  => Map {'type' => 'application/atom+xml', 'title' => '', 'link' => $content},
            'icon'                  => Map {'type' => 'image/x-icon', 'rel' => 'icon', 'link' => $content},
        };

        if (isset($metaTypes[$type])) {
            $attributes = $metaTypes[$type]->setAll($attributes);
        } else {
            $attributes['name'] = $type;
            $attributes['content'] = $content;
        }

        return $this->tag('meta', Map {
            'attr' => $this->attributes($attributes)
        });
    }

    /**
     * Create a script element to include a JS file or to encompass JS code.
     *
     * @param string $source
     * @param bool $isBlock
     * @return string
     */
    public function script(string $source, bool $isBlock = false): string {
        $attributes = Map {'type' => 'text/javascript'};
        $content = '';

        if ($isBlock) {
            $content = '<![CDATA[' . $source . ']]>';
        } else {
            $attributes['src'] = $source;
        }

        return $this->tag('script', Map {
            'attr' => $this->attributes($attributes),
            'body' => $content
        });
    }

    /**
     * Create a style element to encompass CSS.
     *
     * @param string $content
     * @return string
     */
    public function style(string $content): string {
        return $this->tag('style', Map {
            'attr' => $this->attributes(Map {'type' => 'text/css'}),
            'body' => $content
        });
    }

    /**
     * Grab the page title if it has been set.
     *
     * @param string $separator
     * @return string
     */
    public function title(string $separator = ' - '): string {
        $pageTitle = $this->getView()->getVariable('pageTitle');

        if (is_traversable($pageTitle)) {
            return implode($separator, $pageTitle);
        }

        return $pageTitle;
    }

}