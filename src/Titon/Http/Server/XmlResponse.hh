<?hh // strict
/**
 * @copyright   2010-2014, The Titon Project
 * @license     http://opensource.org/licenses/bsd-license.php
 * @link        http://titon.io
 */

namespace Titon\Http\Server;

use Psr\Http\Message\StreamableInterface;
use Titon\Http\Http;
use Titon\Http\Stream\MemoryStream;
use Titon\Utility\Converter;

/**
 * Output XML as the response by converting any type of resource to XML.
 *
 * @package Titon\Http\Server
 */
class XmlResponse extends Response {

    /**
     * Set the body, status code, and optional XML root node.
     * Convert the body to XML before passing along.
     *
     * @param mixed $body
     * @param int $status
     * @param string $root
     */
    public function __construct(mixed $body = null, int $status = Http::OK, string $root = 'root') {
        if (!$body instanceof StreamableInterface) {
            $body = new MemoryStream(Converter::toXml($body, $root));
        }

        parent::__construct($body, $status);
    }

    /**
     * Set the content type and length before sending.
     *
     * @return string
     */
    public function send() {
        $this
            ->contentType('xml')
            ->contentLength($this->getBody()->getSize());

        return parent::send();
    }

}