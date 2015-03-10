<?hh // strict
/**
 * @copyright   2010-2015, The Titon Project
 * @license     http://opensource.org/licenses/bsd-license.php
 * @link        http://titon.io
 */

namespace Titon\Console;

use Titon\Console\Input;
use Titon\Console\Output;
use Titon\Console\InputDefinition\Flag;
use ReflectionClass;

/**
 * The `Console` class bootstraps and handles Input and Output to process and
 * run necessary commands.
 *
 * @package Titon\Console
 */
class Console {

    /**
     * The `Command` object to run
     *
     * @var \Titon\Console\Command|null
     */
    protected ?Command $command;

    /**
     * The `Input` object used to retrieve parsed parameters and commands.
     *
     * @var \Titon\Console\Input
     */
    protected Input $input;

    /**
     * The `Output` object used to send response data to the user.
     *
     * @var \Titon\Console\Output
     */
    protected Output $output;

    /**
     * Construct a new `Console` application.
     */
    public function __construct(?Input $input = null, ?Output $output = null) {
        if (is_null($input)) {
            $input = new Input();
        }

        if (is_null($output)) {
            $output = new Output();
        }

        $this->input = $input;
        $this->output = $output;
    }

    /**
     * Add a `Command` to the application to be parsed by the `Input`.
     *
     * @param Command $command  The `Command` object to add
     *
     * @return $this
     */
    public function addCommand(string $command): this {
        $command = new ReflectionClass($command);
        $command = $command->newInstance($this->input, $this->output);

        $this->input->addCommand($command);

        return $this;
    }

    /**
     * Bootstrap the `Console` application with default parameters and global
     * settings.
     */
    protected function bootstrap(): void {
        /*
         * Add global flags
         */
        $this->input->addFlag((new Flag('help', 'Display this help screen.'))
            ->alias('h'));
        $this->input->addFlag((new Flag('quiet', 'Suppress all output.'))
            ->alias('q'));
        $this->input->addFlag((new Flag('verbose', 'Set the verbosity of the application\'s output.'))
            ->alias('v')
            ->setStackable(true));

        /*
         * Add default styles
         */
        $this->output->setStyle('info', new StyleDefinition('green'));
        $this->output->setStyle('warning', new StyleDefinition('yellow'));
        $this->output->setStyle('error', new StyleDefinition('red'));
    }

    /**
     * Run the `Console` application.
     */
    public function run(): void {
        $this->bootstrap();
        $this->command = $this->input->getActiveCommand();
        if (is_null($this->command)) {
            $this->input->parse();
            $this->renderHelpScreen();
        } else {
            $this->runCommand($this->command);
        }
    }

    /**
     * Register and run the `Command` object.
     *
     * @param Command $command  The `Command` to run
     */
    public function runCommand(Command $command): void {
        $command->registerInput();
        $this->input->parse();

        $flag = $this->input->getFlag('help');

        if ($flag->getValue() === 1) {
            $this->renderHelpScreen($command);
            return;
        }

        $flag = $this->input->getFlag('quiet');

        $verbositySet = false;

        if ($flag->exists()) {
            $verbositySet = true;
            $this->output->setVerbosity(0);
        }

        if ($verbositySet === false) {
            $flag = $this->input->getFlag('verbose');
            $verbosity = $flag->getValue(1);

            invariant(!is_null($verbosity), "Must not be null.");

            $this->output->setVerbosity($verbosity);
        }

        $command->run();
    }

    /**
     * Render the help screen for the application or the `Command` passed in.
     *
     * @param \Titon\Console\Command|null $command  The `Command` to render usage for
     */
    public function renderHelpScreen(?Command $command = null): void {
        $helpScreen = new HelpScreen($this->input);
        if (!is_null($command)) {
            $helpScreen->setCommand($command);
        }

        $this->output->out($helpScreen->render());
    }
}